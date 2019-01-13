/*
 *    player.cpp:
 *
 *    Copyright (C) 2017-2018 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#include "player.hpp"
#include "player_impl.hpp"

namespace lp {

Player::Player(Executor* ui_executor)
{
    impl_ = reinterpret_cast<void*>(new std::shared_ptr<PlayerImpl>(std::make_unique<PlayerImpl>(ui_executor)));
}

Player::~Player()
{
    auto& player_impl = *reinterpret_cast<std::shared_ptr<PlayerImpl>*>(impl_);
    player_impl->SetEventListener(nullptr);
    player_impl->SetRenderer(nullptr);
    player_impl->Close();
    delete reinterpret_cast<std::shared_ptr<PlayerImpl>*>(impl_);
}

auto Player::Open(const std::string& url) -> PlayerOperationResult
{
    return (*reinterpret_cast<std::shared_ptr<PlayerImpl>*>(impl_))->Open(url);
}

auto Player::Play() -> PlayerOperationResult
{
    return (*reinterpret_cast<std::shared_ptr<PlayerImpl>*>(impl_))->Play();
}

auto Player::Pause() -> PlayerOperationResult
{
    return (*reinterpret_cast<std::shared_ptr<PlayerImpl>*>(impl_))->Pause();
}

auto Player::Seek(std::uint64_t time_us) -> PlayerOperationResult
{
    return (*reinterpret_cast<std::shared_ptr<PlayerImpl>*>(impl_))->Seek(time_us);
}

auto Player::Close() -> PlayerOperationResult
{
    return (*reinterpret_cast<std::shared_ptr<PlayerImpl>*>(impl_))->Close();
}

auto Player::GetState() const noexcept -> PlayerState
{
    return (*reinterpret_cast<std::shared_ptr<PlayerImpl>*>(impl_))->GetState();
}

auto Player::GetDuration() const noexcept -> std::int64_t
{
    return (*reinterpret_cast<std::shared_ptr<PlayerImpl>*>(impl_))->GetDuration();
}

auto Player::GetPosition() const noexcept->std::int64_t
{
    return (*reinterpret_cast<std::shared_ptr<PlayerImpl>*>(impl_))->GetPosition();
}

auto Player::SetRenderer(Renderer* renderer) -> void
{
    (*reinterpret_cast<std::shared_ptr<PlayerImpl>*>(impl_))->SetRenderer(renderer);
}

auto Player::SetEventListener(PlayerEventListener* event_listener) -> void
{
    (*reinterpret_cast<std::shared_ptr<PlayerImpl>*>(impl_))->SetEventListener(event_listener);
}

auto Player::SetRenderOptions(const RenderOptions& options) -> bool
{
    return (*reinterpret_cast<std::shared_ptr<PlayerImpl>*>(impl_))->SetRenderOptions(options);
}

void Player::Initialize()
{
    av_register_all();
}

} // namespace lp
