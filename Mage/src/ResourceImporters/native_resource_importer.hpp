#pragma once

#include "resource_importer.hpp"

#include <string_view>


namespace sorcery::mage {
class NativeResourceImporter final : public ResourceImporter {
  RTTR_ENABLE(ResourceImporter)

public:
  auto GetSupportedFileExtensions(std::pmr::vector<std::string>& out) -> void override;
  [[nodiscard]] auto Import(std::filesystem::path const& src,
                            std::vector<ResourceImportResult>& results) -> bool override;
  [[nodiscard]] auto IsNativeImporter() const noexcept -> bool override;
};
}
