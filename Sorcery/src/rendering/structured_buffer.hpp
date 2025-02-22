#pragma once

#include "graphics.hpp"
#include "render_manager.hpp"
#include "../Util.hpp"

#include <span>


namespace sorcery::rendering {
template<typename T> requires (AlignsTo(sizeof(T), 16ull))
class StructuredBuffer {
public:
  [[nodiscard]] static auto New(graphics::GraphicsDevice& device, RenderManager& render_manager,
                                bool cpu_accessible, bool shader_resource = true,
                                bool unordered_access = false) -> StructuredBuffer;
  [[nodiscard]] static auto New(graphics::GraphicsDevice& device, RenderManager& render_manager,
                                std::span<T const> data, bool shader_resource = true,
                                bool unordered_access = false) -> StructuredBuffer;

  StructuredBuffer() = default;

  [[nodiscard]] auto GetBuffer() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Buffer> const&;
  [[nodiscard]] auto GetData() const noexcept -> std::span<T>;
  [[nodiscard]] auto GetSize() const -> UINT;
  [[nodiscard]] auto GetCapacity() const -> UINT;
  auto Resize(UINT new_size) -> void;

private:
  StructuredBuffer(graphics::GraphicsDevice& device, RenderManager& render_manager, UINT initial_capacity,
                   bool cpu_accessible, bool shader_resource, bool unordered_access);
  StructuredBuffer(graphics::GraphicsDevice& device, RenderManager& render_manager, std::span<T const> data,
                   bool shader_resource, bool unordered_access);

  auto RecreateBuffer() -> void;

  graphics::GraphicsDevice* device_{nullptr};
  RenderManager* render_manager_{nullptr};
  graphics::SharedDeviceChildHandle<graphics::Buffer> buffer_;
  T* mapped_ptr_{nullptr};
  UINT capacity_{0};
  UINT size_{0};
  bool cpu_accessible_{false};
  bool srv_{false};
  bool uav_{false};
};
}


#include "structured_buffer.inl"
