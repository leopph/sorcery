#include "render_material.hpp"

#include "constant_buffer.hpp"
#include "shaders/shader_interop.h"


namespace sorcery::rendering {
RenderMaterial::RenderMaterial(wand::GraphicsDevice& device) :
  cb_{CreateConstantBuffer<ShaderMaterial>(device)} {}


auto RenderMaterial::GetBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const& {
  return cb_;
}


auto RenderMaterial::GetRevision() const -> std::uint64_t {
  return revision_;
}


auto RenderMaterial::SetRevision(std::uint64_t const rev) -> void {
  revision_ = rev;
}
}
