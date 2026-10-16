#pragma once

#include <sorcery/host/assets/material_asset.hpp>
#include <sorcery/host/assets/asset_package.hpp>


namespace sorcery {
struct MaterialImportSettings {};


struct MaterialImportResult {
  std::vector<std::byte> bytes;
  rttr::type runtime_type;
  AssetPackagePayloadKind payload_kind;
};


[[nodiscard]] SORCERYAPI
auto ImportMaterial(
  MaterialAssetData const& src,
  MaterialImportSettings const& settings
) -> MaterialImportResult;
}
