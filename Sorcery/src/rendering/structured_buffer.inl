#pragma once


namespace sorcery::rendering {
template<typename T> requires (sizeof(T) % 16 == 0)
auto StructuredBuffer<T>::New(graphics::GraphicsDevice& device, RenderManager& render_manager,
                              bool const cpu_accessible) -> StructuredBuffer {
  return StructuredBuffer{device, render_manager, cpu_accessible};
}


template<typename T> requires (sizeof(T) % 16 == 0)
auto StructuredBuffer<T>::GetBuffer() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return buffer_;
}


template<typename T> requires (sizeof(T) % 16 == 0)
auto StructuredBuffer<T>::GetData() const noexcept -> std::span<T> {
  return std::span<T>{mapped_ptr_, size_};
}


template<typename T> requires (sizeof(T) % 16 == 0)
auto StructuredBuffer<T>::GetSize() const -> UINT {
  return size_;
}


template<typename T> requires (sizeof(T) % 16 == 0)
auto StructuredBuffer<T>::GetCapacity() const -> UINT {
  return capacity_;
}


template<typename T> requires (sizeof(T) % 16 == 0)
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


template<typename T> requires (sizeof(T) % 16 == 0)
StructuredBuffer<T>::StructuredBuffer(graphics::GraphicsDevice& device, RenderManager& render_manager,
                                      bool const cpu_accessible) :
  device_{&device},
  render_manager_{&render_manager},
  cpu_accessible_{cpu_accessible} {
  RecreateBuffer();
}


template<typename T> requires (sizeof(T) % 16 == 0)
auto StructuredBuffer<T>::RecreateBuffer() -> void {
  if (buffer_) {
    render_manager_->KeepAliveWhileInUse(buffer_);
  }

  buffer_ = device_->CreateBuffer(graphics::BufferDesc{
    static_cast<UINT>(capacity_ * sizeof(T)), sizeof(T), false, true, false
  }, cpu_accessible_ ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT);

  mapped_ptr_ = static_cast<T*>(cpu_accessible_ ? buffer_->Map() : nullptr);
}
}
