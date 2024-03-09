#pragma once

#include "graphics.hpp"
#include "../Util.hpp"

#include <cstring>
#include <optional>
#include <utility>


namespace sorcery {
template<typename T>
class ConstantBuffer {
public:
  static auto New(graphics::GraphicsDevice& device) -> std::optional<ConstantBuffer>;

  auto Update(T const& val) -> void;
  [[nodiscard]] auto GetBuffer() const -> graphics::Buffer*;

private:
  ConstantBuffer(graphics::UniqueHandle<graphics::Buffer> buffer, T* ptr);

  graphics::UniqueHandle<graphics::Buffer> buffer_;
  void* ptr_;
};


template<typename T>
auto ConstantBuffer<T>::New(graphics::GraphicsDevice& device) -> std::optional<ConstantBuffer> {
  auto buf{
    device.CreateBuffer(graphics::BufferDesc{RoundToNextMultiple(sizeof(T), 256), 0, true, false, false},
      D3D12_HEAP_TYPE_UPLOAD)
  };

  if (!buf.IsValid()) {
    return std::nullopt;
  }

  auto const ptr{buf->Map()};

  if (!ptr) {
    return std::nullopt;
  }

  return ConstantBuffer{std::move(buf), ptr};
}


template<typename T>
auto ConstantBuffer<T>::Update(T const& val) -> void {
  std::memcpy(ptr_, &val, sizeof(T));
}


template<typename T>
auto ConstantBuffer<T>::GetBuffer() const -> graphics::Buffer* {
  return buffer_.Get();
}


template<typename T>
ConstantBuffer<T>::ConstantBuffer(graphics::UniqueHandle<graphics::Buffer> buffer, T* ptr) :
  buffer_{std::move(buffer)},
  ptr_{ptr} {}
}
