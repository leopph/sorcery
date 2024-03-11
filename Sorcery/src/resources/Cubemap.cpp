#include "Cubemap.hpp"

#include "../Rendering/Renderer.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Cubemap>{"Cubemap"};
}


namespace sorcery {
Cubemap::Cubemap(ID3D11Texture2D& tex, ID3D11ShaderResourceView& srv) noexcept :
  mTex{&tex},
  mSrv{&srv} {
  mTex->AddRef();
  mSrv->AddRef();
}


auto Cubemap::GetTex() const noexcept -> graphics::Buffer* {
  return mSrv.Get();
}
}
