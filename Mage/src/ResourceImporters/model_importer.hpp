#pragma once

#include "resource_importer.hpp"


namespace sorcery::mage {
class ModelImporter final : public ResourceImporter {
  RTTR_ENABLE(ResourceImporter)
  RTTR_REGISTRATION_FRIEND

public:
  auto GetSupportedFileExtensions(
    std::pmr::vector<std::string>& out
  ) -> void override;

  [[nodiscard]]
  auto Import(
    std::filesystem::path const& src,
    std::vector<ResourceImportResult>& results
  ) -> bool override;

private:
  bool fuse_submeshes_{false};
  bool force_idx32_{false};
  bool import_materials_{true};
  bool import_textures_{true};
};
}
