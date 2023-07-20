#pragma once

#include "ResourceImporter.hpp"


namespace sorcery {
class MaterialImporter final : public ResourceImporter {
  RTTR_ENABLE(ResourceImporter)

public:
  auto GetSupportedFileExtensions(std::vector<std::string>& out) -> void override;
  [[nodiscard]] auto Import(std::filesystem::path const& src) -> std::shared_ptr<Resource> override;
  [[nodiscard]] auto GetPrecedence() const noexcept -> int override;
};
}
