#pragma once


namespace sorcery::rendering {
template<typename T> requires (AlignsTo(sizeof(T), 16ull))
auto StructuredBuffer<T>::New(graphics::GraphicsDevice& device, RenderManager& render_manager,
                              bool const cpu_accessible, bool const shader_resource,
                              bool const unordered_access) -> StructuredBuffer {
  return StructuredBuffer{device, render_manager, 1, cpu_accessible, shader_resource, unordered_access};
}


template<typename T> requires (AlignsTo(sizeof(T), 16ull))
auto StructuredBuffer<T>::New(graphics::GraphicsDevice& device, RenderManager& render_manager,
                              std::span<T const> const data, bool const cpu_accessible, bool const shader_resource,
                              bool const unordered_access) -> StructuredBuffer {
  return StructuredBuffer(device, render_manager, data, cpu_accessible, shader_resource, unordered_access);
}


template<typename T> requires (AlignsTo(sizeof(T), 16ull))
auto StructuredBuffer<T>::GetBuffer() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return buffer_;
}


template<typename T> requires (AlignsTo(sizeof(T), 16ull))
auto StructuredBuffer<T>::GetData() const noexcept -> std::span<T> {
  return std::span<T>{mapped_ptr_, size_};
}


template<typename T> requires (AlignsTo(sizeof(T), 16ull))
auto StructuredBuffer<T>::GetSize() const -> UINT {
  return size_;
}


template<typename T> requires (AlignsTo(sizeof(T), 16ull))
auto StructuredBuffer<T>::GetCapacity() const -> UINT {
  return capacity_;
}


template<typename T> requires (AlignsTo(sizeof(T), 16ull))
auto StructuredBuffer<T>::Resize(UINT const new_size) -> void {
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


template<typename T> requires (AlignsTo(sizeof(T), 16ull))
StructuredBuffer<T>::StructuredBuffer(graphics::GraphicsDevice& device, RenderManager& render_manager,
                                      UINT const initial_capacity, bool const cpu_accessible,
                                      bool const shader_resource, bool const unordered_access) :
  device_{&device},
  render_manager_{&render_manager},
  capacity_{initial_capacity},
  cpu_accessible_{cpu_accessible},
  srv_{shader_resource},
  uav_{unordered_access} {
  RecreateBuffer();
}


template<typename T> requires (AlignsTo(sizeof(T), 16ull))
StructuredBuffer<T>::StructuredBuffer(graphics::GraphicsDevice& device, RenderManager& render_manager,
                                      std::span<T const> const data, bool const cpu_accessible,
                                      bool const shader_resource, bool const unordered_access) :
  StructuredBuffer{
    device, render_manager, static_cast<UINT>(data.size()), cpu_accessible, shader_resource, unordered_access
  } {
  Resize(static_cast<UINT>(data.size()));

  if (cpu_accessible_) {
    std::ranges::copy(data.subspan(0, static_cast<UINT>(data.size())), mapped_ptr_);
  } else {
    render_manager.UpdateBuffer(*buffer_, 0, as_bytes(data));
  }
}


template<typename T> requires (AlignsTo(sizeof(T), 16ull))
auto StructuredBuffer<T>::RecreateBuffer() -> void {
  if (buffer_) {
    render_manager_->KeepAliveWhileInUse(buffer_);
  }

  buffer_ = device_->CreateBuffer(graphics::BufferDesc{
    static_cast<UINT>(capacity_ * sizeof(T)), sizeof(T), false, srv_, uav_
  }, cpu_accessible_ ? graphics::CpuAccess::kWrite : graphics::CpuAccess::kNone);

  mapped_ptr_ = static_cast<T*>(cpu_accessible_ ? buffer_->Map() : nullptr);
}
}
