#pragma once

#include "graphics.hpp"
#include "render_manager.hpp"

#include <span>


namespace sorcery::rendering {
template<typename T> requires (sizeof(T) % 16 == 0)
class StructuredBuffer {
public:
  [[nodiscard]] static auto New(graphics::GraphicsDevice& device, RenderManager& render_manager,
                                bool cpu_accessible) -> StructuredBuffer;

  StructuredBuffer() = default;

  [[nodiscard]] auto GetBuffer() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Buffer> const&;
  [[nodiscard]] auto GetData() const noexcept -> std::span<T>;
  [[nodiscard]] auto GetSize() const -> UINT;
  [[nodiscard]] auto GetCapacity() const -> UINT;
  auto Resize(UINT new_size) -> void;

private:
  StructuredBuffer(graphics::GraphicsDevice& device, RenderManager& render_manager, bool cpu_accessible);

  auto RecreateBuffer() -> void;

  graphics::GraphicsDevice* device_{nullptr};
  RenderManager* render_manager_{nullptr};
  graphics::SharedDeviceChildHandle<graphics::Buffer> buffer_;
  T* mapped_ptr_{nullptr};
  UINT capacity_{1};
  UINT size_{0};
  bool cpu_accessible_{false};
};
}


#include "structured_buffer.inl"
