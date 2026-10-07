#pragma once

#include "../util.hpp"


namespace sorcery::rendering {
constexpr static auto kConstantBufferSizeRounding = 256;


template<typename T>
MappedConstantBuffer<T>::MappedConstantBuffer(wand::GraphicsDevice& device, T const* data) {
  auto const size = static_cast<UINT>(RoundToNextMultiple(sizeof(T), kConstantBufferSizeRounding));
  buffer_view_ = CreateBufferWithView(device, wand::BufferDesc{
    .size = size,
    .usage = wand::BufferUsage::kConstantBuffer
  }, wand::BufferViewDesc{
    .offset = 0,
    .size = size,
    .stride = 0,
    .usage = wand::BufferViewUsage::kConstantBuffer
  }, wand::CpuAccess::kWrite);

  ptr_ = static_cast<T*>(buffer_view_->GetBuffer()->Map());

  if (data) {
    *ptr_ = *data;
  }
}


template<typename T>
auto MappedConstantBuffer<T>::GetBufferView() const -> wand::SharedDeviceHandle<wand::BufferView> const& {
  return buffer_view_;
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
auto CreateConstantBuffer(wand::GraphicsDevice& device) -> wand::SharedDeviceHandle<wand::BufferView> {
  auto const size = static_cast<UINT>(RoundToNextMultiple(sizeof(T), kConstantBufferSizeRounding));

  return CreateBufferWithView(device, wand::BufferDesc{
    .size = size,
    .usage = wand::BufferUsage::kConstantBuffer | wand::BufferUsage::kCopyDestination
  }, wand::BufferViewDesc{
    .offset = 0,
    .size = size,
    .stride = 0,
    .usage = wand::BufferViewUsage::kConstantBuffer
  }, wand::CpuAccess::kNone);
}
}
