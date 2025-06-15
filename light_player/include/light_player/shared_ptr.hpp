/*
 *    shared_ptr.hpp:
 *
 *    Copyright (C) 2025 Light Lin <lxrite@gmail.com> All Rights Reserved.
 *
 */

#ifndef LIGHT_PLAYER_SHARE_PTR_HPP
#define LIGHT_PLAYER_SHARE_PTR_HPP

#include <utility>

#include "include/light_player/ref_counted.hpp"

namespace lp {

    template <typename T>
    class SharedPtr {
    public:
        SharedPtr() : ptr_(nullptr) {}

        SharedPtr(T* ptr, bool add_ref) : ptr_(ptr) {
            if (add_ref && ptr_) {
                ptr_->AddRef();
            }
        }

        SharedPtr(const SharedPtr<T>& other) : ptr_(other.ptr_) {
            if (ptr_) {
                ptr_->AddRef();
            }
        }

        SharedPtr(SharedPtr<T>&& other) : ptr_(other.ptr_) {
            other.ptr_ = nullptr;
        }

        ~SharedPtr() {
            if (ptr_) {
                ptr_->Release();
            }
        }

        SharedPtr<T>& operator=(const SharedPtr<T>& other) {
            if (this != &other) {
                if (other.ptr_) {
                    other.ptr_->AddRef();
                }
                if (ptr_) {
                    ptr_->Release();
                }
                ptr_ = other.ptr_;
            }
            return *this;
        }

        SharedPtr<T>& operator=(SharedPtr<T>&& other) {
            if (this != &other) {
                if (ptr_) {
                    ptr_->Release();
                }
                ptr_ = other.ptr_;
                other.ptr_ = nullptr;
            }
            return *this;
        }

        T* get() const { return ptr_; }
        T& operator*() const { return *ptr_; }
        T* operator->() const { return ptr_; }

        explicit operator bool() const { return ptr_ != nullptr; }

    private:
        T* ptr_;
    };

    template <typename T, typename... Args>
    SharedPtr<T> MakeShared(Args&&... args) {
        return SharedPtr<T>(new T(std::forward<Args>(args)...), false);
    }

} // namespace lp

#endif // LIGHT_PLAYER_SHARE_PTR_HPP
