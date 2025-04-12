#pragma once

#include "ResourceImporter.hpp"


namespace sorcery {
class MeshImporter final : public ResourceImporter {
  RTTR_ENABLE(ResourceImporter)
  RTTR_REGISTRATION_FRIEND

public:
  auto GetSupportedFileExtensions(std::pmr::vector<std::string>& out) -> void override;
  [[nodiscard]] auto Import(std::filesystem::path const& src, std::vector<std::byte>& bytes,
                            ExternalResourceCategory& categ) -> bool override;
  [[nodiscard]] auto GetImportedType(std::filesystem::path const& resPathAbs) noexcept -> rttr::type override;

private:
  bool fuse_submeshes_{false};
  bool force_idx32_{false};
};
}
