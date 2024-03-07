#pragma once

#include "graphics.hpp"

#include <cassert>
#include <span>


namespace sorcery {
template<typename T>
class StructuredBuffer {
  static_assert(sizeof(T) % 16 == 0, "StructuredBuffer contained type must have a size divisible by 16.");

  graphics::GraphicsDevice* device_;
  graphics::UniqueHandle<graphics::Buffer> buffer_;
  T* mapped_ptr_{nullptr};
  int capacity_{1};
  int size_{0};

  auto RecreateBuffer() -> void;

public:
  explicit StructuredBuffer(graphics::GraphicsDevice* device);

  [[nodiscard]] auto GetBuffer() const noexcept -> graphics::Buffer*;
  [[nodiscard]] auto GetData() const noexcept -> std::span<T>;
  auto Resize(int new_size) -> void;
};


template<typename T>
auto StructuredBuffer<T>::RecreateBuffer() -> void {
  buffer_ = device_->CreateBuffer(graphics::BufferDesc{capacity_ * sizeof(T), sizeof(T), false, true, false});
  mapped_ptr_ = buffer_->Map();
}


template<typename T>
StructuredBuffer<T>::StructuredBuffer(graphics::GraphicsDevice* const device):
  device_{device} {
  RecreateBuffer();
}


template<typename T>
auto StructuredBuffer<T>::GetBuffer() const noexcept -> graphics::Buffer* {
  return buffer_.Get();
}


template<typename T>
auto StructuredBuffer<T>::GetData() const noexcept -> std::span<T> {
  return std::span<T>{mapped_ptr_, size_};
}


template<typename T>
auto StructuredBuffer<T>::Resize(int const new_size) -> void {
  assert(!mapped_ptr_);

  auto new_capacity = capacity_;

  while (new_capacity < new_size) {
    new_capacity *= 2;
  }

  if (new_capacity != capacity_) {
    capacity_ = new_capacity;
    size_ = new_size;
    RecreateBuffer();
  } else if (size_ != new_size) {
    size_ = new_size;
  }
}
}
