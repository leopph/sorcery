#pragma once

#include <algorithm>

#include "wand/flags.hpp"


namespace sorcery::rendering {
template<typename T>
MappedStructuredBuffer<T>::MappedStructuredBuffer(wand::GraphicsDevice& device, std::uint64_t const element_count,
                                                  bool const shader_resource, bool const unordered_access) :
  device_{&device},
  srv_{shader_resource},
  uav_{unordered_access} {
  Reallocate(element_count);
}


template<typename T>
MappedStructuredBuffer<T>::MappedStructuredBuffer(wand::GraphicsDevice& device, std::span<T const> data,
                                                  bool shader_resource, bool unordered_access) :
  MappedStructuredBuffer{device, data.size(), shader_resource, unordered_access} {
  std::ranges::copy(data, std::ranges::begin(data_));
}


template<typename T>
auto MappedStructuredBuffer<T>::GetBufferView() const -> wand::SharedDeviceHandle<wand::BufferView> const& {
  return buffer_view_;
}


template<typename T>
auto MappedStructuredBuffer<T>::GetData() -> std::span<T> {
  return data_;
}


template<typename T>
auto MappedStructuredBuffer<T>::GetData() const -> std::span<T const> {
  return data_;
}


template<typename T>
auto MappedStructuredBuffer<T>::GetElementCount() const -> std::uint64_t {
  return element_count_;
}


template<typename T>
auto MappedStructuredBuffer<T>::Reallocate(
  std::uint64_t const element_count) -> wand::SharedDeviceHandle<wand::BufferView> {
  if (element_count_ == element_count) {
    return nullptr;
  }

  element_count_ = element_count;
  auto const old_buf = buffer_view_;

  if (element_count == 0) {
    buffer_view_.reset();
    data_ = std::span<T>{};
    return old_buf;
  }

  auto buf_usage{wand::BufferUsage::kCopySource};
  auto view_usage{wand::BufferViewUsage::kNone};

  if (srv_) {
    buf_usage |= wand::BufferUsage::kShaderResource;
    view_usage |= wand::BufferViewUsage::kShaderResource;
  }

  if (uav_) {
    buf_usage |= wand::BufferUsage::kUnorderedAccess;
    view_usage |= wand::BufferViewUsage::kUnorderedAccess;
  }

  buffer_view_ = CreateBufferWithView(*device_, wand::BufferDesc{
    .size = element_count * sizeof(T),
    .usage = buf_usage
  }, wand::BufferViewDesc{
    .offset = 0,
    .size = element_count * sizeof(T),
    .stride = sizeof(T),
    .usage = view_usage
  }, wand::CpuAccess::kWrite);

  data_ = std::span<T>{static_cast<T*>(buffer_view_->GetBuffer()->Map()), static_cast<std::size_t>(element_count)};

  return old_buf;
}


template<typename T>
auto CreateStructuredBuffer(wand::GraphicsDevice& device, std::uint64_t const element_count, bool const shader_resource,
                            bool const unordered_access) -> wand::SharedDeviceHandle<wand::BufferView> {
  auto buf_usage = wand::BufferUsage::kCopySource | wand::BufferUsage::kCopyDestination;
  auto view_usage = wand::BufferViewUsage::kNone;

  if (shader_resource) {
    buf_usage |= wand::BufferUsage::kShaderResource;
    view_usage |= wand::BufferViewUsage::kShaderResource;
  }

  if (unordered_access) {
    buf_usage |= wand::BufferUsage::kUnorderedAccess;
    view_usage |= wand::BufferViewUsage::kUnorderedAccess;
  }

  return CreateBufferWithView(device, wand::BufferDesc{
    .size = element_count * sizeof(T),
    .usage = buf_usage
  }, wand::BufferViewDesc{
    .offset = 0,
    .size = element_count * sizeof(T),
    .stride = sizeof(T),
    .usage = view_usage
  }, wand::CpuAccess::kNone);
}
}
