/*
 *    player_impl.hpp:
 *
 *    Copyright (C) 2017-2025 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_PLAYER_IMPL_HPP
#define LIGHT_PLAYER_PLAYER_IMPL_HPP

#include <atomic>
#include <mutex>
#include <optional>
#include <thread>

extern "C" {
#include <libavformat/avformat.h>
}

#include "include/light_player/shared_ptr.hpp"
#include "include/light_player/executor.hpp"
#include "include/light_player/io/stream.hpp"
#include "include/light_player/player_state.hpp"
#include "include/light_player/player_event_listener.hpp"
#include "include/light_player/renderer.hpp"

#include "src/clock.hpp"
#include "src/coroutine/co_condition_variable.hpp"
#include "src/coroutine/fire_and_forget.hpp"
#include "src/decoder.hpp"
#include "src/demuxer.hpp"
#include "src/io/custom_io_wrapper.hpp"
#include "src/queue.hpp"

using lp::coroutine::CoConditionVariable;
using lp::coroutine::FireAndForget;
using lp::coroutine::Task;

namespace lp {

    class PlayerImpl : public std::enable_shared_from_this<PlayerImpl> {
    public:
        PlayerImpl(Executor* ui_executor);
        ~PlayerImpl();
        auto Open(const std::string& url) -> PlayerOperationResult;
        auto OpenStream(io::IStream* stream) -> PlayerOperationResult;
        auto Play() -> PlayerOperationResult;
        auto Pause() -> PlayerOperationResult;
        auto Seek(std::uint64_t time_us) -> PlayerOperationResult;
        auto Close() -> PlayerOperationResult;

        auto GetState() const noexcept -> PlayerState;
        auto GetDuration() const noexcept -> std::int64_t;
        auto GetPosition() const noexcept -> std::int64_t;

        auto SetRenderer(Renderer* renderer) -> void;
        auto SetEventListener(PlayerEventListener* event_listener) -> void;

    private:
        auto Open(const std::string& url, const std::shared_ptr<io::CustomIOWrapper>& custom_io) ->lp::PlayerOperationResult;
        auto DoOpen(const std::string& url, const std::shared_ptr<io::CustomIOWrapper>& custom_io) -> void;
        auto RunReadTask() -> FireAndForget;
        auto ReadTask() -> Task<void>;
        auto RunDecodeAudioTask() -> FireAndForget;
        auto DecodeAudioTask() -> Task<void>;
        auto RunDecodeVideoTask() -> FireAndForget;
        auto DecodeVideoTask() -> Task<void>;
        auto RunRenderTask() -> FireAndForget;
        auto RenderTask() -> Task<void>;
        auto RunAudioTask() -> FireAndForget;
        auto AudioTask() -> Task<void>;
        auto Reset() -> void;

    private:
        auto VideoFrameDuration(const Frame& frame, const Frame& next_frame) const -> double;
        auto ComputeVideoFrameTargetDelay(double delay) const -> double;
        auto IsCloseRequested(bool lock) const -> bool;
        auto OnThreadExit() -> void;
        auto OnAudioServiceDrained() -> void;

    private:
        PlayerState player_state_ = PlayerState::Ready;
        SharedPtr <Executor> ui_executor_;
        PlayerEventListener* event_listener_ = nullptr;
        mutable std::mutex mutex_;
        int video_stream_index_ = -1;
        int audio_stream_index_ = -1;
        std::shared_ptr<Demuxer> demuxer_;
        std::shared_ptr<Decoder> video_decoder_;
        std::shared_ptr<Decoder> audio_decoder_;
        PacketQueue video_pkt_queue_;
        PacketQueue audio_pkt_queue_;
        CoConditionVariable read_pkt_cv_;
        FrameQueue video_frame_queue_;
        FrameQueue audio_frame_queue_;
        CoConditionVariable video_render_cv_;
        CoConditionVariable audio_output_cv_;
        CoConditionVariable video_decode_cv_;
        CoConditionVariable audio_decode_cv_;
        std::shared_ptr<Clock> video_clock_;
        std::shared_ptr<Clock> audio_clock_;
        AVFormatContext* format_ctx_ = nullptr;
        SharedPtr<Renderer> renderer_;
        std::thread render_thread_;
        AVPixelFormat video_pix_fmt_;
        std::optional<Frame> last_show_frame_;
        double max_frame_duration_ = 10.0;
        double frame_timer_ = 0.0;
        bool paused_ = false;
        bool is_seek_requested_ = false;
        std::int64_t seek_target_us_ = 0;
        bool is_close_requested_ = false;
        std::atomic<int> running_thread_cnt_ = 0;
        bool is_end_of_file_ = false;
        bool is_end_of_video_stream_ = false;
        bool is_end_of_audio_stream_ = false;
        std::int64_t position_ = 0;
        bool use_audio_position_ = false;
        std::shared_ptr<io::CustomIOWrapper> custom_io_;
    };

} // namespace lp

#endif // LIGHT_PLAYER_PLAYER_IMPL_HPP
