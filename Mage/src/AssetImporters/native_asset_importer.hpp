#pragma once

#include "asset_importer.hpp"

#include <string_view>


namespace sorcery::mage {
class NativeAssetImporter final : public AssetImporter {
  RTTR_ENABLE(AssetImporter)

public:
  auto GetSupportedFileExtensions(std::pmr::vector<std::string>& out) -> void override;
  [[nodiscard]] auto Import(std::filesystem::path const& src,
                            std::vector<AssetImportResult>& results) -> bool override;
  [[nodiscard]] auto IsNativeImporter() const noexcept -> bool override;
};
}
