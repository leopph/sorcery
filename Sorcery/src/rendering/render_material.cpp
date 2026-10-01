#include "render_material.hpp"


namespace sorcery::rendering {
RenderMaterial::RenderMaterial(wand::GraphicsDevice& device) :
  cb_{ConstantBuffer<ShaderMaterial>::New(device, false).value()} {}


auto RenderMaterial::GetBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const& {
  return cb_.GetBuffer();
}


auto RenderMaterial::GetRevision() const -> std::uint64_t {
  return revision_;
}


auto RenderMaterial::SetRevision(std::uint64_t const rev) -> void {
  revision_ = rev;
}
}
