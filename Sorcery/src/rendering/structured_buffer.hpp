#pragma once

#include "graphics.hpp"

#include <cassert>
#include <span>


namespace sorcery {
template<typename T>
class StructuredBuffer {
public:
  static auto New(graphics::GraphicsDevice& device, bool cpu_accessible) -> StructuredBuffer;

  StructuredBuffer() = default;

  [[nodiscard]] auto GetBuffer() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Buffer> const&;
  [[nodiscard]] auto GetData() const noexcept -> std::span<T>;
  auto Resize(UINT new_size) -> void;

private:
  StructuredBuffer(graphics::GraphicsDevice& device, bool cpu_accessible);

  auto RecreateBuffer() -> void;

  graphics::GraphicsDevice* device_{nullptr};
  graphics::SharedDeviceChildHandle<graphics::Buffer> buffer_;
  T* mapped_ptr_{nullptr};
  UINT capacity_{1};
  UINT size_{0};
  bool cpu_accessible_{false};

  static_assert(sizeof(T) % 16 == 0, "StructuredBuffer contained type must have a size divisible by 16.");
};


template<typename T>
auto StructuredBuffer<T>::New(graphics::GraphicsDevice& device, bool const cpu_accessible) -> StructuredBuffer {
  return StructuredBuffer{device, cpu_accessible};
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


template<typename T>
StructuredBuffer<T>::StructuredBuffer(graphics::GraphicsDevice& device, bool cpu_accessible) :
  device_{&device},
  cpu_accessible_{cpu_accessible} {
  RecreateBuffer();
}


template<typename T>
auto StructuredBuffer<T>::RecreateBuffer() -> void {
  buffer_ = device_->CreateBuffer(graphics::BufferDesc{
    static_cast<UINT>(capacity_ * sizeof(T)), sizeof(T), false, true, false
  }, cpu_accessible_ ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT);
  mapped_ptr_ = static_cast<T*>(buffer_->Map());
}
}
