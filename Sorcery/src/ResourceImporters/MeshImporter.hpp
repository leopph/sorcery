#pragma once

#include "ResourceImporter.hpp"


namespace sorcery {
class MeshImporter final : public ResourceImporter {
  RTTR_ENABLE(ResourceImporter)

public:
  LEOPPHAPI auto GetSupportedFileExtensions(std::vector<std::string>& out) -> void override;
  [[nodiscard]] LEOPPHAPI auto Import(std::filesystem::path const& src) -> ObserverPtr<Resource> override;
  [[nodiscard]] LEOPPHAPI auto GetPrecedence() const noexcept -> int override;
  [[nodiscard]] LEOPPHAPI auto GetImportedType(std::filesystem::path const& resPathAbs) noexcept -> rttr::type override;
};
}
