#include "material_import.hpp"

#include <ranges>

#include "../Reflection.hpp"
#include "../resources/Material.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::MaterialImportSettings>("Material Import Settings")
    .constructor<>()(rttr::policy::ctor::as_object);
}


namespace sorcery {
auto ImportMaterial(
  MaterialResourceData const& src,
  [[maybe_unused]] MaterialImportSettings const& settings
) -> MaterialImportResult {
  auto const yaml_node{SerializeMaterialResourceData(src, ResourceRefSerialization::kLocal)};
  auto const yaml_str{YAML::Dump(yaml_node)};

  std::vector<std::byte> mtl_bytes;
  mtl_bytes.resize(yaml_str.size());
  std::ranges::copy(yaml_str | std::views::transform([](char c) { return static_cast<std::byte>(c); }),
    mtl_bytes.begin());

  return MaterialImportResult{std::move(mtl_bytes), rttr::type::get<Material>(), ResourcePackagePayloadKind::kMaterial};
}
}
