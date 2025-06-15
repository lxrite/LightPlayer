/*
 *    audio_service_audio_graph.cpp:
 *
 *    Copyright (C) 2025 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.h>
#include <winrt/Windows.Media.Audio.h>
#include <winrt/Windows.Media.MediaProperties.h>
#include <winrt/Windows.Media.Render.h>

#include "src/ports/windows/uwp/audio/audio_service_audio_graph.hpp"

using namespace lp::ports::windows::uwp::audio;
using namespace winrt::impl;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Media;
using namespace winrt::Windows::Media::Audio;
using namespace winrt::Windows::Media::MediaProperties;
using namespace winrt::Windows::Media::Render;

AudioService_AudioGraph::AudioService_AudioGraph(AudioGraph graph, AudioFrameInputNode input_node, AudioDeviceOutputNode output_node, std::function<void()> audio_frame_completed)
	: audio_graph_(graph), input_node_(input_node), output_node_(output_node)
{
	input_node_.AudioFrameCompleted([this, audio_frame_completed](AudioFrameInputNode node, AudioFrameCompletedEventArgs e) {
		queued_freme_count_.fetch_sub(1);
		if (audio_frame_completed) {
			audio_frame_completed();
		}
	});
}

auto AudioService_AudioGraph::Start() -> void
{
	audio_graph_.Start();
}

auto AudioService_AudioGraph::Stop() -> void
{
	audio_graph_.Stop();
}

auto AudioService_AudioGraph::WriteAudioFrameData(std::uint8_t* data, std::size_t size) -> void
{
	AudioFrame frame(size);
	auto buffer = frame.LockBuffer(AudioBufferAccessMode::Write);
	auto buffer_ref = buffer.CreateReference();
	std::uint8_t* write_buf;
	std::uint32_t write_buf_size;
	buffer_ref.as<IMemoryBufferByteAccess>()->GetBuffer(&write_buf, &write_buf_size);
	std::memcpy(write_buf, data, size);
	buffer_ref.Close();
	buffer.Close();
	// A maximum of 64 frames can be queued in the AudioFrameInputNode at one time. Attempting to queue more than 64 frames will result in an error.
	++queued_freme_count_;
	input_node_.AddFrame(frame);
}

auto AudioService_AudioGraph::QueuedSampleCount() -> std::uint32_t const
{
	return input_node_.QueuedSampleCount();
}

auto AudioService_AudioGraph::QueuedFrameCount() -> std::uint32_t const
{
	auto count = queued_freme_count_.load();
	if (count < 0) {
		return 0;
	}
	return static_cast<std::uint32_t>(count);
}

auto AudioService_AudioGraph::CreatePCMAudioService(std::uint32_t sample_rate, std::uint32_t channel_count, std::uint32_t bits_per_sample, std::function<void()> audio_frame_completed) -> Task<std::shared_ptr<AudioService_AudioGraph>>
{
	auto aep = AudioEncodingProperties::CreatePcm(sample_rate, channel_count, bits_per_sample);
	AudioGraphSettings settings(AudioRenderCategory::Media);
	settings.EncodingProperties(aep);
	auto createResult = co_await AudioGraph::CreateAsync(settings);
	if (createResult.Status() != AudioGraphCreationStatus::Success) {
		co_return nullptr;
	}
	auto audioGraph = createResult.Graph();
	auto frameInputNode = audioGraph.CreateFrameInputNode(aep);
		
	auto createOutputNodeResult = co_await audioGraph.CreateDeviceOutputNodeAsync();
	if (createOutputNodeResult.Status() != AudioDeviceNodeCreationStatus::Success) {
		co_return nullptr;
	}
	auto deviceOutputNode = createOutputNodeResult.DeviceOutputNode();

	frameInputNode.AddOutgoingConnection(deviceOutputNode);
	co_return std::make_shared<AudioService_AudioGraph>(audioGraph, frameInputNode, deviceOutputNode, audio_frame_completed);
}
