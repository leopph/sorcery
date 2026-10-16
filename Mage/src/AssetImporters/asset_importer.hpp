#pragma once

#include "sorcery/reflection.hpp"
#include "sorcery/host/assets/asset_package.hpp"

#include <filesystem>
#include <string>
#include <vector>


namespace sorcery::mage {
class AssetImporter {
  RTTR_ENABLE()

public:
  // The importer must be able to provide a list of file extensions it supports.
  // The file extension format is expected to be ".ext" (including the dot).
  virtual auto GetSupportedFileExtensions(std::pmr::vector<std::string>& out) -> void = 0;

  // Imports the asset file at the given path and stores the results in the output parameters.
  // Returns whether the import was successful.
  [[nodiscard]] virtual auto Import(std::filesystem::path const& src,
                                    std::vector<AssetImportResult>& results) -> bool = 0;

  // Specifies whether this importer imports assets as-is or if it produces a custom binary.
  [[nodiscard]] virtual auto IsNativeImporter() const noexcept -> bool;

  AssetImporter() = default;
  AssetImporter(AssetImporter const& other) = default;
  AssetImporter(AssetImporter&& other) noexcept = default;

  virtual ~AssetImporter() = default;

  auto operator=(AssetImporter const& other) -> AssetImporter& = default;
  auto operator=(AssetImporter&& other) noexcept -> AssetImporter& = default;
};
}
