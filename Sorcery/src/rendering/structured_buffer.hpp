#pragma once

#include "graphics.hpp"

#include <cassert>
#include <span>


namespace sorcery {
template<typename T>
class StructuredBuffer {
  static_assert(sizeof(T) % 16 == 0, "StructuredBuffer contained type must have a size divisible by 16.");

  auto RecreateBuffer() -> void;

public:
  static auto New(graphics::GraphicsDevice& device) -> StructuredBuffer;

  StructuredBuffer() = default;

  [[nodiscard]] auto GetBuffer() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Buffer> const&;
  [[nodiscard]] auto GetData() const noexcept -> std::span<T>;
  auto Resize(UINT new_size) -> void;

private:
  graphics::GraphicsDevice* device_{nullptr};
  graphics::SharedDeviceChildHandle<graphics::Buffer> buffer_;
  T* mapped_ptr_{nullptr};
  UINT capacity_{1};
  UINT size_{0};
};


template<typename T>
auto StructuredBuffer<T>::RecreateBuffer() -> void {
  buffer_ = device_->CreateBuffer(graphics::BufferDesc{
    static_cast<UINT>(capacity_ * sizeof(T)), sizeof(T), false, true, false
  }, D3D12_HEAP_TYPE_UPLOAD);
  mapped_ptr_ = static_cast<T*>(buffer_->Map());
}


template<typename T>
auto StructuredBuffer<T>::New(graphics::GraphicsDevice& device) -> StructuredBuffer {
  StructuredBuffer sb;
  sb.device_ = &device;
  sb.RecreateBuffer();
  return sb;
}


template<typename T>
auto StructuredBuffer<T>::GetBuffer() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return buffer_;
}


template<typename T>
auto StructuredBuffer<T>::GetData() const noexcept -> std::span<T> {
  return std::span<T>{mapped_ptr_, size_};
}


template<typename T>
auto StructuredBuffer<T>::Resize(UINT const new_size) -> void {
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
