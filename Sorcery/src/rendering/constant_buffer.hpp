#pragma once

#include "graphics.hpp"
#include "../Util.hpp"

#include <cstring>
#include <optional>
#include <utility>


namespace sorcery::rendering {
template<typename T>
class ConstantBuffer {
public:
  [[nodiscard]] static auto New(graphics::GraphicsDevice& device, bool cpu_accessible) -> std::optional<ConstantBuffer>;

  ConstantBuffer() = default;

  auto Update(T const& val) -> void;
  [[nodiscard]] auto GetBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const&;

private:
  ConstantBuffer(graphics::SharedDeviceChildHandle<graphics::Buffer> buffer, T* ptr);

  graphics::SharedDeviceChildHandle<graphics::Buffer> buffer_;
  void* ptr_{nullptr};
};


template<typename T>
auto ConstantBuffer<T>::New(graphics::GraphicsDevice& device,
                            bool const cpu_accessible) -> std::optional<ConstantBuffer> {
  auto buf{
    device.CreateBuffer(graphics::BufferDesc{
      static_cast<UINT>(RoundToNextMultiple(sizeof(T), 256)), 0, true, false, false
    }, cpu_accessible ? graphics::CpuAccess::kWrite : graphics::CpuAccess::kNone)
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
auto ConstantBuffer<T>::GetBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return buffer_;
}


template<typename T>
ConstantBuffer<T>::ConstantBuffer(graphics::SharedDeviceChildHandle<graphics::Buffer> buffer, T* ptr) :
  buffer_{std::move(buffer)},
  ptr_{ptr} {}
}
