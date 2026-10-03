#pragma once

#include "wand/wand.hpp"


namespace sorcery::rendering {
template<typename T>
class MappedConstantBuffer {
public:
  explicit MappedConstantBuffer(wand::GraphicsDevice& device, T const* data = nullptr);

  [[nodiscard]]
  auto GetBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const&;
  [[nodiscard]]
  auto GetData() -> T&;
  [[nodiscard]]
  auto GetData() const -> T const&;

private:
  wand::SharedDeviceChildHandle<wand::Buffer> buffer_{};
  T* ptr_{nullptr};
};


template<typename T>
[[nodiscard]]
auto CreateConstantBuffer(wand::GraphicsDevice& device) -> wand::SharedDeviceChildHandle<wand::Buffer>;
}


#include "constant_buffer.inl"
