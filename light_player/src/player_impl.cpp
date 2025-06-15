/*
 *    player_impl.cpp:
 *
 *    Copyright (C) 2017-2025 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#include <algorithm>
#include <cassert>
#include <chrono>
#include <future>
#include <vector>
#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include "include/light_player/picture.hpp"
#include "src/audio/audio_service.hpp"
#include "src/player_impl.hpp"

static const double kLightPlayerRefreshRate = 0.01;
static const double kLightPlayerAVSyncThresholdMax = 0.1;
static const double kLightPlayerAVSyncThresholdMin = 0.04;
static const double kLightPlayerAVSyncFramedupThreshold = 0.1;

namespace lp {

PlayerImpl::PlayerImpl(Executor* ui_executor)
    : ui_executor_(ui_executor, true)
{
    video_clock_ = std::make_shared<Clock>(video_pkt_queue_.SerialPtr());
    audio_clock_ = std::make_shared<Clock>(audio_pkt_queue_.SerialPtr());
}

PlayerImpl::~PlayerImpl()
{
    Reset();
}

auto PlayerImpl::Open(const std::string& url) -> PlayerOperationResult
{
    return Open(url, nullptr);
}

auto PlayerImpl::OpenStream(io::IStream* stream) -> PlayerOperationResult
{
    auto custom_io = std::make_shared<io::CustomIOWrapper>(stream);
    return Open("", custom_io);
}

auto PlayerImpl::Play() -> PlayerOperationResult
{
    if (player_state_ == PlayerState::Playing) {
        return PlayerOperationResult::Done;
    }
    if (player_state_ != PlayerState::Paused) {
        return PlayerOperationResult::Fail;
    }
    {
        std::unique_lock<std::mutex> lck(mutex_);
        paused_ = false;
    }
    audio_output_cv_.NotifyOne();
    video_render_cv_.NotifyOne();
    player_state_ = PlayerState::Playing;
    if (event_listener_) {
        PlayerStatePlayingEventArg event_arg;
        event_listener_->OnPlayerStatePlaying(event_arg);
    }
    return PlayerOperationResult::Done;
}

auto PlayerImpl::Pause() -> PlayerOperationResult
{
    if (player_state_ == PlayerState::Paused) {
        return PlayerOperationResult::Done;
    }
    if (player_state_ != PlayerState::Playing) {
        return PlayerOperationResult::Fail;
    }
    {
        std::unique_lock<std::mutex> lck(mutex_);
        paused_ = true;
    }
    audio_output_cv_.NotifyOne();
    video_render_cv_.NotifyOne();
    player_state_ = PlayerState::Paused;
    if (event_listener_) {
        PlayerStatePausedEventArg event_arg;
        event_listener_->OnPlayerStatePaused(event_arg);
    }
    return PlayerOperationResult::Done;
}

auto PlayerImpl::Seek(std::uint64_t time_us) -> PlayerOperationResult
{
    if (player_state_ != PlayerState::Playing &&
        player_state_ != PlayerState::Paused) {
        return PlayerOperationResult::Fail;
    }
    auto res = PlayerOperationResult::Fail;
    {
        std::unique_lock<std::mutex> lck(mutex_);
        if (!is_seek_requested_) {
            res = PlayerOperationResult::Pending;
            is_seek_requested_ = true;
            seek_target_us_ = time_us;
        }
    }
    return res;
}

auto PlayerImpl::Close() -> PlayerOperationResult
{
    if (player_state_ == PlayerState::Ready) {
        return PlayerOperationResult::Done;
    }
    else if (player_state_ == PlayerState::Closing) {
        return PlayerOperationResult::Pending;
    }
    {
        std::unique_lock<std::mutex> lck(mutex_);
        is_close_requested_ = true;
    }
    read_pkt_cv_.NotifyOne();
    audio_decode_cv_.NotifyOne();
    video_decode_cv_.NotifyOne();
    audio_output_cv_.NotifyOne();
    video_render_cv_.NotifyOne();
    player_state_ = PlayerState::Closing;
    return PlayerOperationResult::Pending;
}

auto PlayerImpl::GetState() const noexcept -> PlayerState
{
    return player_state_;
}

auto PlayerImpl::GetDuration() const noexcept -> std::int64_t
{
    auto state = GetState();
    if (state == PlayerState::Opended ||
        state == PlayerState::Playing ||
        state == PlayerState::Paused) {
        static_assert(1000000 == AV_TIME_BASE);
        return format_ctx_->duration;
    }
    return 0;
}

auto PlayerImpl::GetPosition() const noexcept -> std::int64_t
{
    auto state = GetState();
    if (state == PlayerState::Opended ||
        state == PlayerState::Playing ||
        state == PlayerState::Paused) {
        std::lock_guard<std::mutex> lck(mutex_);
        return position_;
    }
    return 0;
}

auto PlayerImpl::SetRenderer(Renderer* renderer) -> void
{
    std::lock_guard<std::mutex> lck(mutex_);
    renderer_ = SharedPtr<Renderer>(renderer, true);
}

auto PlayerImpl::SetEventListener(PlayerEventListener* event_listener) -> void
{
    event_listener_ = event_listener;
}

auto PlayerImpl::Open(const std::string& url, const std::shared_ptr<io::CustomIOWrapper>& custom_io) -> lp::PlayerOperationResult
{
    if (player_state_ != PlayerState::Ready) {
        return PlayerOperationResult::Fail;
    }

    auto url_copy = url;
    auto custom_io_copy = custom_io;
    // TODO(Light Lin):
    auto self = shared_from_this();
    ++running_thread_cnt_;
    std::thread([this, self, url_copy, custom_io_copy]() {
        DoOpen(url_copy, custom_io_copy);
        OnThreadExit();
        }).detach();
    return PlayerOperationResult::Pending;
}

auto PlayerImpl::DoOpen(const std::string& url, const std::shared_ptr<io::CustomIOWrapper>& custom_io) -> void
{
    if (IsCloseRequested(true)) {
        return;
    }
    auto self = shared_from_this();
    ui_executor_->Dispatch([this, self]() {
        if (player_state_ != PlayerState::Ready) {
            return;
        }
        player_state_ = PlayerState::Opening;
        if (event_listener_) {
            PlayerStateOpeningEventArg event_arg;
            event_listener_->OnPlayerStateOpening(event_arg);
        }
    });

    bool open_success = false;
    do
    {
        format_ctx_ = avformat_alloc_context();
        if (format_ctx_ == nullptr) {
            break;
        }
        if (custom_io) {
            format_ctx_->pb = custom_io->IOContext();
            custom_io_ = custom_io;
        }
        if (avformat_open_input(&format_ctx_, url.c_str(), nullptr, nullptr) != 0) {
            break;
        }
        if (IsCloseRequested(true)) {
            return;
        }
        if (avformat_find_stream_info(format_ctx_, nullptr) != 0) {
            break;
        }
        if (IsCloseRequested(true)) {
            return;
        }

        audio_stream_index_ = av_find_best_stream(format_ctx_, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
        video_stream_index_ = av_find_best_stream(format_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);

        use_audio_position_ = true;
        if (audio_stream_index_ >= 0) {
            auto audio_codecpar = format_ctx_->streams[audio_stream_index_]->codecpar;
            auto audio_codec = avcodec_find_decoder(audio_codecpar->codec_id);
            if (audio_codec != nullptr) {
                std::promise<std::shared_ptr<Decoder>> p;
                auto f = p.get_future();
                ui_executor_->Dispatch([&p, audio_codecpar, audio_codec]() {
                    auto audio_decoder = Decoder::OpenDecoder(audio_codecpar, audio_codec, nullptr);
                    p.set_value(audio_decoder);
                });
                audio_decoder_ = f.get();
            }
        }

        if (video_stream_index_ >= 0) {
            auto video_codecpar = format_ctx_->streams[video_stream_index_]->codecpar;
            video_pix_fmt_ = static_cast<AVPixelFormat>(video_codecpar->format);
            auto video_codec = avcodec_find_decoder(video_codecpar->codec_id);
            if (video_codec != nullptr) {
                std::promise<std::shared_ptr<Decoder>> p;
                auto f = p.get_future();
                ui_executor_->Dispatch([&p, video_codecpar, video_codec]() {
                    auto video_decoder = Decoder::OpenDecoder(video_codecpar, video_codec, nullptr);
                    p.set_value(video_decoder);
                });
                video_decoder_ = f.get();
            }
            use_audio_position_ = false;
        }

        if (audio_decoder_ == nullptr && video_decoder_ == nullptr) {
            break;
        }

        demuxer_ = std::make_shared<Demuxer>(format_ctx_);
        max_frame_duration_ = (format_ctx_->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;

        ui_executor_->Dispatch([this, self]() {
            if (player_state_ != PlayerState::Opening) {
                return;
            }
            player_state_ = PlayerState::Opended;
            if (event_listener_) {
                PlayerStateOpenedEventArg event_arg;
                event_listener_->OnPlayerStateOpened(event_arg);
            }
            if (player_state_ != PlayerState::Opended) {
                return;
            }
            if (paused_) {
                player_state_ = PlayerState::Paused;
                if (event_listener_) {
                    PlayerStatePausedEventArg event_arg;
                    event_listener_->OnPlayerStatePaused(event_arg);
                }
            }
            else {
                player_state_ = PlayerState::Playing;
                if (event_listener_) {
                    PlayerStatePlayingEventArg event_arg;
                    event_listener_->OnPlayerStatePlaying(event_arg);
                }
            }
        });

        RunReadTask();

        if (audio_decoder_) {
            RunDecodeAudioTask();
            RunAudioTask();
        }

        if (video_decoder_) {
            RunDecodeVideoTask();
            RunRenderTask();
        }

        open_success = true;
    } while (false);
}

auto PlayerImpl::RunReadTask() -> FireAndForget
{
    auto self = shared_from_this();
    ++running_thread_cnt_;
    co_await ReadTask();
    OnThreadExit();
}

auto PlayerImpl::ReadTask() -> Task<void>
{
    co_await ThreadPool::Shared().Schedule();
    while (true) {
        bool is_close_requested = false;
        std::int64_t seek_target_us = 0;
        bool is_seek_requested = false;
        {
            std::unique_lock<std::mutex> lck(mutex_);
            is_close_requested = IsCloseRequested(false);
            is_seek_requested = is_seek_requested_;
            seek_target_us = seek_target_us_;
        }
        if (is_close_requested) {
            co_return;
        }
        if (is_seek_requested) {
            using seek_tb = std::ratio<1000000, AV_TIME_BASE>::type;
            int64_t seek_target = static_cast<std::int64_t>(static_cast<double>(seek_target_us) * static_cast<double>(seek_tb::num) / static_cast<double>(seek_tb::den));
            int64_t seek_min = INT64_MIN;
            int64_t seek_max = INT64_MAX;
            int seek_ret = avformat_seek_file(format_ctx_, -1, seek_min, seek_target, seek_max, 0);
            if (seek_ret < 0) {
                // TODO(Light Lin):
            }
            else {
                std::unique_lock<std::mutex> lck(mutex_);
                if (audio_stream_index_ >= 0) {
                    audio_pkt_queue_.Flush();
                    audio_decode_cv_.NotifyOne();
                }
                if (video_stream_index_ >= 0) {
                    video_pkt_queue_.Flush();
                    video_decode_cv_.NotifyOne();
                }
            }
            is_seek_requested_ = false;
            seek_target_us_ = 0;
            // TODO(Light Lin):
        }
        Packet packet;
        auto get_packet_err = demuxer_->GetPacket(packet);
        if (get_packet_err == DemuxErrors::Ok) {
            if (packet.RawPacketPtr()->stream_index == video_stream_index_) {
                std::unique_lock<std::mutex> lck(mutex_);
                packet.SetSerial(video_pkt_queue_.Serial());
                video_pkt_queue_.Push(std::move(packet));
                video_decode_cv_.NotifyOne();
            }
            else if (packet.RawPacketPtr()->stream_index == audio_stream_index_) {
                std::unique_lock<std::mutex> lck(mutex_);
                packet.SetSerial(audio_pkt_queue_.Serial());
                audio_pkt_queue_.Push(std::move(packet));
                audio_decode_cv_.NotifyOne();
            }
            {
                std::unique_lock<std::mutex> lck(mutex_);
                co_await read_pkt_cv_.Wait(lck, [this]() -> bool {
                    return IsCloseRequested(false) || (video_stream_index_ >= 0 && video_pkt_queue_.Size() < 10) || (audio_stream_index_ >= 0 && audio_pkt_queue_.Size() < 10);
                });
            }
        }
        else if (get_packet_err == DemuxErrors::EndOfFile) {
            std::unique_lock<std::mutex> lck(mutex_);
            is_end_of_file_ = true;
            if (audio_stream_index_ >= 0) {
                auto null_pkt = Packet::MakeNullPacket();
                null_pkt.SetSerial(audio_pkt_queue_.Serial());
                audio_pkt_queue_.Push(null_pkt);
                audio_decode_cv_.NotifyOne();
            }
            if (video_stream_index_ >= 0) {
                auto null_pkt = Packet::MakeNullPacket();
                null_pkt.SetSerial(video_pkt_queue_.Serial());
                video_pkt_queue_.Push(null_pkt);
                video_decode_cv_.NotifyOne();
            }
            break;
        }
        else {
            // TODO(Light Lin):
            std::lock_guard<std::mutex> lck(mutex_);
            is_end_of_file_ = true;
            if (audio_stream_index_ >= 0) {
                auto null_pkt = Packet::MakeNullPacket();
                null_pkt.SetSerial(audio_pkt_queue_.Serial());
                audio_pkt_queue_.Push(null_pkt);
                audio_decode_cv_.NotifyOne();
            }
            if (video_stream_index_ >= 0) {
                auto null_pkt = Packet::MakeNullPacket();
                null_pkt.SetSerial(video_pkt_queue_.Serial());
                video_pkt_queue_.Push(null_pkt);
                video_decode_cv_.NotifyOne();
            }
            break;
        }
    }
}

auto PlayerImpl::RunDecodeAudioTask() -> FireAndForget
{
    auto self = shared_from_this();
    ++running_thread_cnt_;
    co_await DecodeAudioTask();
    OnThreadExit();
}

auto PlayerImpl::DecodeAudioTask() -> Task<void>
{
    co_await ThreadPool::Shared().Schedule();
    auto time_base = format_ctx_->streams[audio_stream_index_]->time_base;
    while (true) {
        if (IsCloseRequested(true)) {
            co_return;
        }
        auto frame = Frame{};
        auto receive_frame_err = audio_decoder_->ReceiveFrame(frame);
        if (receive_frame_err == DecodeErrors::Ok) {
            // TODO(Light Lin):
            auto duration = av_q2d(AVRational{ frame.RawFramePtr()->nb_samples, frame.RawFramePtr()->sample_rate });
            auto pts = (frame.RawFramePtr()->pts == AV_NOPTS_VALUE) ? NAN : frame.RawFramePtr()->pts * av_q2d(time_base);
            frame.SetSerial(audio_decoder_->PacketSerial().value());
            frame.SetDuration(duration);
            frame.SetPts(pts);
            {
                auto lck = std::unique_lock<std::mutex>(mutex_);
                audio_frame_queue_.Push(std::move(frame));
                audio_output_cv_.NotifyOne();
                co_await audio_decode_cv_.Wait(lck, [this]() -> bool {
                    return IsCloseRequested(false) || audio_frame_queue_.Size() < 10;
                });
            }
        }
        else if (receive_frame_err == DecodeErrors::Again) {
            {
                auto lck = std::unique_lock<std::mutex>(mutex_);
                read_pkt_cv_.NotifyOne();
                co_await audio_decode_cv_.Wait(lck, [this]() -> bool {
                    return IsCloseRequested(false) || !audio_pkt_queue_.Empty();
                });
                if (IsCloseRequested(false)) {
                    co_return;
                }
                auto packet = audio_pkt_queue_.Pop();
                int packet_serial = packet.Serial().value();
                audio_decoder_->SetPacketSerial(packet_serial);
                if (packet.IsFlushPacket()) {
                    audio_decoder_->FlushBuffers();
                }
                if (packet.IsFlushPacket() || audio_decoder_->PacketSerial().value() != audio_pkt_queue_.Serial()) {
                    continue;
                }
                audio_decoder_->SendPacket(packet);
                continue;
            }
        }
        else if (receive_frame_err == DecodeErrors::EndOfFile) {
            std::unique_lock<std::mutex> lck(mutex_);
            co_await audio_decode_cv_.Wait(lck, [this]() -> bool {
                return IsCloseRequested(false) || audio_frame_queue_.Empty();
            });
            if (IsCloseRequested(false)) {
                break;
            }
            is_end_of_audio_stream_ = true;
            if (video_stream_index_ < 0 || is_end_of_video_stream_) {
                auto self = shared_from_this();
                lck.unlock();
                ui_executor_->Dispatch([this, self]() {
                    // TODO(Light Lin):
                    player_state_ = PlayerState::Ended;
                    if (event_listener_) {
                        PlayerStateMediaEndEventArg event_arg;
                        event_listener_->OnPlayerStateMediaEnd(event_arg);
                    }
                });
            }
            // TODO(Light Lin):
            break;
        }
        else {
            // TODO(Light Lin):
            break;
        }
    }
}

auto PlayerImpl::RunDecodeVideoTask() -> FireAndForget
{
    auto self = shared_from_this();
    ++running_thread_cnt_;
    co_await DecodeVideoTask();
    OnThreadExit();
}

auto PlayerImpl::DecodeVideoTask() -> Task<void>
{
    co_await ThreadPool::Shared().Schedule();
    AVRational frame_rate = av_guess_frame_rate(format_ctx_, format_ctx_->streams[video_stream_index_], nullptr);
    auto time_base = format_ctx_->streams[video_stream_index_]->time_base;
    while (true) {
        if (IsCloseRequested(true)) {
            co_return;
        }
        Frame frame;
        auto receive_frame_err = video_decoder_->ReceiveFrame(frame);
        if (receive_frame_err == DecodeErrors::Ok) {
            auto duration = (frame_rate.num && frame_rate.den ? av_q2d(AVRational { frame_rate.den, frame_rate.num }) : 0);
            auto pts = (frame.RawFramePtr()->pts == AV_NOPTS_VALUE) ? NAN : frame.RawFramePtr()->pts * av_q2d(time_base);
            frame.SetSerial(video_decoder_->PacketSerial().value());
            frame.SetDuration(duration);
            frame.SetPts(pts);
            {
                auto lck = std::unique_lock<std::mutex>(mutex_);
                video_frame_queue_.Push(std::move(frame));
                video_render_cv_.NotifyOne();
                co_await video_decode_cv_.Wait(lck, [this]() -> bool {
                    return IsCloseRequested(false) || video_frame_queue_.Size() < 10;
                });
            }
        }
        else if (receive_frame_err == DecodeErrors::Again) {
            auto lck = std::unique_lock<std::mutex>(mutex_);
            read_pkt_cv_.NotifyOne();
            co_await video_decode_cv_.Wait(lck, [this]() -> bool {
                return IsCloseRequested(false) || !video_pkt_queue_.Empty();
            });
            if (IsCloseRequested(false)) {
                co_return;
            }
            auto packet = video_pkt_queue_.Pop();
            int packet_serial = packet.Serial().value();
            video_decoder_->SetPacketSerial(packet_serial);
            if (packet.IsFlushPacket()) {
                video_decoder_->FlushBuffers();
            }
            if (packet.IsFlushPacket() || video_decoder_->PacketSerial().value() != video_pkt_queue_.Serial()) {
                continue;
            }
            video_decoder_->SendPacket(packet);
            continue;
        }
        else if (receive_frame_err == DecodeErrors::EndOfFile) {
            std::unique_lock<std::mutex> lck(mutex_);
            co_await video_decode_cv_.Wait(lck, [this]() -> bool {
                return IsCloseRequested(false) || video_frame_queue_.Empty();
            });
            if (IsCloseRequested(false)) {
                break;
            }
            is_end_of_video_stream_ = true;
            if (audio_stream_index_ < 0 || is_end_of_audio_stream_) {
                auto self = shared_from_this();
                lck.unlock();
                ui_executor_->Dispatch([this, self]() {
                    // TODO(Light Lin):
                    player_state_ = PlayerState::Ended;
                    if (event_listener_) {
                        PlayerStateMediaEndEventArg event_arg;
                        event_listener_->OnPlayerStateMediaEnd(event_arg);
                    }
                });
            }
            // TODO(Light Lin):
            break;
        }
        else {
            // TODO(Light Lin):
            break;
        }
    }
}

auto PlayerImpl::RunRenderTask() -> FireAndForget
{
    auto self = shared_from_this();
    ++running_thread_cnt_;
    co_await RenderTask();
    OnThreadExit();
}

auto PlayerImpl::RenderTask() -> Task<void>
{
    co_await ThreadPool::Shared().Schedule();

    while (true) {
        {
            std::unique_lock<std::mutex> lck(mutex_);
            co_await video_render_cv_.Wait(lck, [this]() {
                return IsCloseRequested(false) || (!paused_ && !video_frame_queue_.Empty());
            });
            if (IsCloseRequested(false)) {
                co_return;
            }
        }

        std::optional<Frame> frame;
        std::optional<int> packet_queue_serial;
        {
            auto lck = std::unique_lock<std::mutex>(mutex_);
            frame = video_frame_queue_.Peek();
            if (frame.has_value())
            {
                packet_queue_serial = std::make_optional<int>(video_pkt_queue_.Serial());
            }
        }
        if (!frame)
        {
            assert(false);
            continue;
        }
        auto last_show_frame = last_show_frame_;
        if (!last_show_frame) {
            last_show_frame = frame;
        }

        if (frame->Serial() != packet_queue_serial) {
            video_frame_queue_.Pop();
            continue;
        }

        if (last_show_frame->Serial() != frame->Serial()) {
            frame_timer_ = Clock::GetTimeRelative();
        }

        auto last_duration = VideoFrameDuration(last_show_frame.value(), frame.value());

        double delay;
        {
            auto lck = std::unique_lock<std::mutex>(mutex_);
            delay = ComputeVideoFrameTargetDelay(last_duration);
        }

        auto remaining_time = kLightPlayerRefreshRate;

        auto time = Clock::GetTimeRelative();
        bool display = true;
        if (time < frame_timer_ + delay) {
            remaining_time = FFMIN(frame_timer_ + delay - time, remaining_time);
            display = false;
        }
        else {
            frame_timer_ += delay;
            if (delay > 0 && time - frame_timer_ > kLightPlayerAVSyncThresholdMax) {
                frame_timer_ = time;
            }

            {
                auto lck = std::unique_lock<std::mutex>(mutex_);
                if (!std::isnan(frame->Pts())) {
                    video_clock_->SetClock(frame->Pts(), frame->Serial().value());
                    if (!use_audio_position_) {
                        position_ = static_cast<std::int64_t>(frame->Pts() * 1000000);
                    }
                }
                last_show_frame_ = video_frame_queue_.Pop();
                video_decode_cv_.NotifyOne();
                auto next_frame = video_frame_queue_.Peek();
                if (next_frame) {
                    auto duration = VideoFrameDuration(*frame, *next_frame);
                    if (time > frame_timer_ + duration) {
                        continue;
                    }
                }
            }
        }

        frame = last_show_frame;
        SharedPtr<Renderer> renderer;
        {
            std::lock_guard<std::mutex> lck(mutex_);
            renderer = renderer_;
        }
        if (display && renderer) {
            lp::Picture picture(frame->RawFramePtr(), true);
            renderer->Render(picture);
        }
        // TODO(Light Lin): Optimize using condition variable
        auto wait_duration = std::chrono::milliseconds(static_cast<std::int64_t>(remaining_time * 1000));
        std::this_thread::sleep_for(wait_duration);
    }
}

auto PlayerImpl::RunAudioTask() -> FireAndForget
{
    auto self = shared_from_this();
    ++running_thread_cnt_;
    co_await AudioTask();
    OnThreadExit();
}

auto PlayerImpl::AudioTask() -> Task<void>
{
    co_await ThreadPool::Shared().Schedule();
    auto audio_codec_ctx_ = audio_decoder_->GetCodecContext();
    AVChannelLayout out_channel_layout;
    av_channel_layout_default(&out_channel_layout, audio_codec_ctx_->ch_layout.nb_channels);
    AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    int out_samples_rate = audio_codec_ctx_->sample_rate;
    AVChannelLayout in_channel_layout;
    av_channel_layout_copy(&in_channel_layout, &out_channel_layout);
    AVSampleFormat in_sample_fmt = audio_codec_ctx_->sample_fmt;
    int in_sample_rate = audio_codec_ctx_->sample_rate;

    std::uint32_t sample_rate = out_samples_rate;
    std::uint32_t channel_count = audio_codec_ctx_->ch_layout.nb_channels;
    std::uint32_t bits_per_sample = av_get_bytes_per_sample(out_sample_fmt) * 8;
    std::weak_ptr<PlayerImpl> wp_this = shared_from_this();
    auto on_drained = [wp_this, this]() {
        auto self = wp_this.lock();
        if (self) {
            self->OnAudioServiceDrained();
        }
    };

    auto audio_service = co_await lp::audio::CreatePCMAudioService(sample_rate, channel_count, bits_per_sample, on_drained);
    if (audio_service == nullptr) {
        co_return;
    }
    audio_service->Start();

    do
    {
        SwrContext* au_convert_ctx = nullptr;
        auto alloc_ctx_result = swr_alloc_set_opts2(&au_convert_ctx, &out_channel_layout, out_sample_fmt, out_samples_rate, &in_channel_layout, in_sample_fmt, in_sample_rate, 0, nullptr);
        if (alloc_ctx_result != 0) {
            break;
        }

        if (swr_init(au_convert_ctx) < 0) {
            break;
        }

        double audio_clock = NAN;
        while (true) {
            {
                auto lck = std::unique_lock<std::mutex>(mutex_);
                co_await audio_output_cv_.Wait(lck, [this, audio_service]() {
                    return IsCloseRequested(false) || (!paused_ && audio_service->QueuedFrameCount() < 5);
                });
                if (IsCloseRequested(false)) {
                    break;
                }
            }

            std::optional<Frame> frame;
            std::optional<int> packet_queue_serial;
            {
                auto lck = std::unique_lock<std::mutex>(mutex_);
                frame = audio_frame_queue_.Pop();
                if (frame.has_value())
                {
                    packet_queue_serial = std::make_optional<int>(video_pkt_queue_.Serial());
                }
                audio_decode_cv_.NotifyOne();
            }
            if (!frame)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<std::int64_t>(kLightPlayerRefreshRate * 100)));
                continue;
            }
            if (!std::isnan(frame->Pts())) {
                audio_clock = frame->Pts() + static_cast<double>(frame->RawFramePtr()->nb_samples) / static_cast<double>(frame->RawFramePtr()->sample_rate);
            }
            uint8_t *out_audio_data_buf = nullptr;
            unsigned int out_audio_data_buf_size = av_samples_alloc(&out_audio_data_buf, nullptr, frame->RawFramePtr()->ch_layout.nb_channels, frame->RawFramePtr()->nb_samples, out_sample_fmt, 0);
            int out_samples_per_channel = swr_convert(au_convert_ctx, &out_audio_data_buf, out_audio_data_buf_size, (const std::uint8_t**)(frame->RawFramePtr()->extended_data), frame->RawFramePtr()->nb_samples);
            int out_len = (unsigned int)(out_samples_per_channel * frame->RawFramePtr()->ch_layout.nb_channels * av_get_bytes_per_sample(out_sample_fmt));
            audio_service->WriteAudioFrameData(out_audio_data_buf, out_len);
            av_free(out_audio_data_buf);

            auto time = Clock::GetTimeRelative();
            if (!std::isnan(audio_clock)) {
                auto lck = std::unique_lock<std::mutex>(mutex_);
                auto pts = audio_clock;
                audio_clock_->SetClockAt(pts, frame->Serial().value(), time);
                if (use_audio_position_) {
                    position_ = static_cast<std::int64_t>(pts * 1000000);
                }
            }
        }
    } while (false);
    audio_service->Stop();
}

auto PlayerImpl::VideoFrameDuration(const Frame& frame, const Frame& next_frame) const -> double
{
    if (frame.Serial().has_value() && frame.Serial() == next_frame.Serial()) {
        double duration = static_cast<double>(next_frame.Pts() - frame.Pts());
        if (std::isnan(duration) || duration <= 0 || duration > max_frame_duration_) {
            return frame.Duration();
        }
        else {
            return duration;
        }
    }
    else {
        return 0.0;
    }
}

auto PlayerImpl::ComputeVideoFrameTargetDelay(double delay) const -> double
{
    auto diff = video_clock_->GetClock() - audio_clock_->GetClock();
    if (!std::isnan(diff)) {
        auto sync_threshold = std::clamp(delay, kLightPlayerAVSyncThresholdMin, kLightPlayerAVSyncThresholdMax);
        if (fabs(diff) < max_frame_duration_) {
            if (diff <= -sync_threshold) {
                delay = FFMAX(0, delay + diff);
            }
            else if (diff >= sync_threshold && delay > kLightPlayerAVSyncFramedupThreshold) {
                delay = delay + diff;
            }
            else if (diff >= sync_threshold) {
                delay = 2 * delay;
            }
        }
    }
    return delay;
}

auto PlayerImpl::IsCloseRequested(bool lock) const -> bool
{
    if (lock) {
        std::unique_lock<std::mutex> lck(mutex_);
        return is_close_requested_;
    }
    return is_close_requested_;
}

auto PlayerImpl::OnThreadExit() -> void
{
    auto self = shared_from_this();
    auto running_thread_cnt = --running_thread_cnt_;
    if (running_thread_cnt == 0) {
        ui_executor_->Dispatch([this, self]() {
            if (player_state_ == PlayerState::Closing) {
                Reset();
                player_state_ = PlayerState::Ready;
                if (event_listener_) {
                    PlayerStateClosedEventArg event_arg;
                    event_listener_->OnPlayerStateClosed(event_arg);
                }
            }
        });
    }
}

auto PlayerImpl::OnAudioServiceDrained() -> void
{
    audio_output_cv_.NotifyOne();
}

auto PlayerImpl::Reset() -> void
{
    is_close_requested_ = false;
    last_show_frame_ = {};
    video_pkt_queue_.Flush();
    audio_pkt_queue_.Flush();
    video_clock_ = std::make_shared<Clock>(video_pkt_queue_.SerialPtr());
    audio_clock_ = std::make_shared<Clock>(audio_pkt_queue_.SerialPtr());
    video_frame_queue_.Clear();
    audio_frame_queue_.Clear();
    demuxer_ = nullptr;
    audio_decoder_ = nullptr;
    video_decoder_ = nullptr;
    use_audio_position_ = false;
    audio_stream_index_ = 1;
    video_stream_index_ = 1;
    if (format_ctx_ != nullptr) {
        avformat_close_input(&format_ctx_);
        avformat_free_context(format_ctx_);
    }
}

} // namespace lp
