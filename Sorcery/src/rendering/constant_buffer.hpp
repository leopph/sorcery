#pragma once

#include "wand/wand.hpp"


namespace sorcery::rendering {
template<typename T>
class MappedConstantBuffer {
public:
  explicit MappedConstantBuffer(wand::GraphicsDevice& device, T const* data = nullptr);

  [[nodiscard]]
  auto GetBufferView() const -> wand::SharedDeviceHandle<wand::BufferView> const&;
  [[nodiscard]]
  auto GetData() -> T&;
  [[nodiscard]]
  auto GetData() const -> T const&;

private:
  wand::SharedDeviceHandle<wand::BufferView> buffer_view_{};
  T* ptr_{nullptr};
};


template<typename T>
[[nodiscard]]
auto CreateConstantBuffer(wand::GraphicsDevice& device) -> wand::SharedDeviceHandle<wand::BufferView>;
}


#include "constant_buffer.inl"
