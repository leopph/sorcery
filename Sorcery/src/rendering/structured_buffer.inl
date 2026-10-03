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
auto MappedStructuredBuffer<T>::GetBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const& {
  return buffer_;
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
  std::uint64_t const element_count) -> wand::SharedDeviceChildHandle<wand::Buffer> {
  if (element_count_ == element_count) {
    return nullptr;
  }

  element_count_ = element_count;
  auto const old_buf = buffer_;

  if (element_count == 0) {
    buffer_.reset();
    data_ = std::span<T>{};
    return old_buf;
  }

  auto usage{wand::BufferUsage::kCopySource};

  if (srv_) {
    usage |= wand::BufferUsage::kShaderResource;
  }

  if (uav_) {
    usage |= wand::BufferUsage::kUnorderedAccess;
  }

  buffer_ = device_->CreateBuffer(wand::BufferDesc{
    .size = element_count * sizeof(T), .stride = sizeof(T), .usage = usage
  }, wand::CpuAccess::kWrite);

  data_ = std::span<T>{static_cast<T*>(buffer_->Map()), static_cast<std::size_t>(element_count)};

  return old_buf;
}


template<typename T>
auto CreateStructuredBuffer(wand::GraphicsDevice& device, std::uint64_t const element_count, bool const shader_resource,
                            bool const unordered_access) -> wand::SharedDeviceChildHandle<wand::Buffer> {
  auto usage{wand::BufferUsage::kCopySource | wand::BufferUsage::kCopyDestination};

  if (shader_resource) {
    usage |= wand::BufferUsage::kShaderResource;
  }

  if (unordered_access) {
    usage |= wand::BufferUsage::kUnorderedAccess;
  }

  return device.CreateBuffer(wand::BufferDesc{
    .size = element_count * sizeof(T), .stride = sizeof(T), .usage = usage
  }, wand::CpuAccess::kNone);
}
}
