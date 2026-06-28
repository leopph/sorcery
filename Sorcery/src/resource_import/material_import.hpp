#pragma once

#include "../material_resource.hpp"
#include "../resource_package.hpp"


namespace sorcery {
struct MaterialImportSettings {};


struct MaterialImportResult {
  std::vector<std::byte> bytes;
  rttr::type runtime_type;
  ResourcePackagePayloadKind payload_kind;
};


[[nodiscard]] SORCERYAPI
auto ImportMaterial(
  MaterialResourceData const& src,
  MaterialImportSettings const& settings
) -> MaterialImportResult;
}
