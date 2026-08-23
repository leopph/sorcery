#pragma once

#include "../Util.hpp"
#include "wand/wand.hpp"

#include <cstring>
#include <optional>
#include <utility>


namespace sorcery::rendering {
template<typename T>
class ConstantBuffer {
public:
  [[nodiscard]] static auto New(wand::GraphicsDevice& device, bool cpu_accessible) -> std::optional<ConstantBuffer>;

  ConstantBuffer() = default;

  auto Update(T const& val) -> void;
  [[nodiscard]] auto GetBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const&;

  [[nodiscard]]
  auto IsValid() const -> bool;

  operator bool() const;

private:
  ConstantBuffer(wand::SharedDeviceChildHandle<wand::Buffer> buffer, T* ptr);

  wand::SharedDeviceChildHandle<wand::Buffer> buffer_;
  void* ptr_{nullptr};
};


template<typename T>
auto ConstantBuffer<T>::New(wand::GraphicsDevice& device,
                            bool const cpu_accessible) -> std::optional<ConstantBuffer> {
  auto buf{
    device.CreateBuffer(wand::BufferDesc{
      static_cast<UINT>(RoundToNextMultiple(sizeof(T), 256)), 0, true, false, false
    }, cpu_accessible ? wand::CpuAccess::kWrite : wand::CpuAccess::kNone)
  };

  if (!buf) {
    return std::nullopt;
  }

  void* ptr{nullptr};

  if (cpu_accessible) {
    ptr = buf->Map();

    if (!ptr) {
      return std::nullopt;
    }
  }

  return ConstantBuffer{std::move(buf), static_cast<T*>(ptr)};
}


template<typename T>
auto ConstantBuffer<T>::Update(T const& val) -> void {
  std::memcpy(ptr_, &val, sizeof(T));
}


template<typename T>
auto ConstantBuffer<T>::GetBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const& {
  return buffer_;
}


template<typename T>
auto ConstantBuffer<T>::IsValid() const -> bool {
  return buffer_.get();
}


template<typename T>
ConstantBuffer<T>::operator bool() const {
  return IsValid();
}


template<typename T>
ConstantBuffer<T>::ConstantBuffer(wand::SharedDeviceChildHandle<wand::Buffer> buffer, T* ptr) :
  buffer_{std::move(buffer)},
  ptr_{ptr} {}
}
