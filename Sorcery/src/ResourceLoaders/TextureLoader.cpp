#include "TextureLoader.hpp"
#include "../FileIo.hpp"
#include "../Renderer.hpp"

#include <directxtk/DDSTextureLoader.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;


namespace sorcery {
auto TextureLoader::Load(std::filesystem::path const& pathAbs) -> MaybeNull<ObserverPtr<Resource>> {
  std::vector<std::uint8_t> bytes;

  if (!ReadFileBinary(pathAbs, bytes)) {
    return nullptr;
  }

  ComPtr<ID3D11Resource> res;
  ComPtr<ID3D11ShaderResourceView> srv;

  if (FAILED(DirectX::CreateDDSTextureFromMemoryEx(gRenderer.GetDevice(), bytes.data(), std::size(bytes), 0, D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, 0, 0, DirectX::DDS_LOADER_DEFAULT, res.GetAddressOf(), srv.GetAddressOf()))) {
    return nullptr;
  }

  D3D11_RESOURCE_DIMENSION resDim;
  res->GetType(&resDim);

  if (resDim == D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
    ComPtr<ID3D11Texture2D> tex2D;
    if (FAILED(res.As(&tex2D))) {
      return nullptr;
    }

    D3D11_TEXTURE2D_DESC desc;
    tex2D->GetDesc(&desc);

    if (desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) {
      return new Cubemap{*tex2D.Get(), *srv.Get()};
    }

    return new Texture2D{*tex2D.Get(), *srv.Get()};
  }

  return nullptr;
}
}
