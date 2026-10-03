#pragma once

#include "../Util.hpp"
#include "wand/flags.hpp"


namespace sorcery::rendering {
constexpr static auto kConstantBufferSizeRounding = 256;


template<typename T>
MappedConstantBuffer<T>::MappedConstantBuffer(wand::GraphicsDevice& device, T const* data) {
  buffer_ = device.CreateBuffer(wand::BufferDesc{
    .size = static_cast<UINT>(RoundToNextMultiple(sizeof(T), kConstantBufferSizeRounding)), .stride = 0,
    .usage = wand::BufferUsage::kConstantBuffer
  }, wand::CpuAccess::kWrite);

  ptr_ = static_cast<T*>(buffer_->Map());

  if (data) {
    *ptr_ = *data;
  }
}


template<typename T>
auto MappedConstantBuffer<T>::GetBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const& {
  return buffer_;
}


template<typename T>
auto MappedConstantBuffer<T>::GetData() -> T& {
  return *ptr_;
}


template<typename T>
auto MappedConstantBuffer<T>::GetData() const -> T const& {
  return *ptr_;
}


template<typename T>
auto CreateConstantBuffer(wand::GraphicsDevice& device) -> wand::SharedDeviceChildHandle<wand::Buffer> {
  return device.CreateBuffer(wand::BufferDesc{
    .size = static_cast<UINT>(RoundToNextMultiple(sizeof(T), kConstantBufferSizeRounding)), .stride = 0,
    .usage = wand::BufferUsage::kConstantBuffer | wand::BufferUsage::kCopyDestination
  }, wand::CpuAccess::kNone);
}
}
