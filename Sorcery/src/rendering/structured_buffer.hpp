#pragma once

#include <cstdint>
#include <span>

#include "../observer_ptr.hpp"
#include "wand/wand.hpp"


namespace sorcery::rendering {
template<typename T>
class MappedStructuredBuffer {
public:
  explicit MappedStructuredBuffer(wand::GraphicsDevice& device, std::uint64_t element_count = 0,
                                  bool shader_resource = true, bool unordered_access = false);
  MappedStructuredBuffer(wand::GraphicsDevice& device, std::span<T const> data, bool shader_resource = true,
                         bool unordered_access = false);

  [[nodiscard]]
  auto GetBufferView() const -> wand::SharedDeviceHandle<wand::BufferView> const&;
  [[nodiscard]]
  auto GetData() -> std::span<T>;
  [[nodiscard]]
  auto GetData() const -> std::span<T const>;
  [[nodiscard]]
  auto GetElementCount() const -> std::uint64_t;

  // Returns the previously allocated buffer view.
  auto Reallocate(std::uint64_t element_count) -> wand::SharedDeviceHandle<wand::BufferView>;

private:
  ObserverPtr<wand::GraphicsDevice> device_;
  wand::SharedDeviceHandle<wand::BufferView> buffer_view_{};
  std::span<T> data_{};
  std::uint64_t element_count_{0};
  bool srv_;
  bool uav_;
};


template<typename T>
[[nodiscard]]
auto CreateStructuredBuffer(
  wand::GraphicsDevice& device,
  std::uint64_t element_count,
  bool shader_resource = true,
  bool unordered_access = false
) -> wand::SharedDeviceHandle<wand::BufferView>;
}


#include "structured_buffer.inl"
