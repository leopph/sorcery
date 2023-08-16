#pragma once

#include "ResourceImporter.hpp"


namespace sorcery {
class MeshImporter final : public ResourceImporter {
  RTTR_ENABLE(ResourceImporter)

public:
  auto GetSupportedFileExtensions(std::vector<std::string>& out) -> void override;
  [[nodiscard]] auto Import(std::filesystem::path const& src, std::vector<std::byte>& bytes) -> bool override;
  [[nodiscard]] auto GetImportedType(std::filesystem::path const& resPathAbs) noexcept -> rttr::type override;
};
}
