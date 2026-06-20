#pragma once

#include "Reflection.hpp"
#include "resource_package.hpp"

#include <filesystem>
#include <string>
#include <vector>


namespace sorcery::mage {
class ResourceImporter {
  RTTR_ENABLE()

public:
  // The importer must be able to provide a list of file extensions it supports.
  // The file extension format is expected to be ".ext" (including the dot).
  virtual auto GetSupportedFileExtensions(std::pmr::vector<std::string>& out) -> void = 0;

  // Imports the resource file at the given path and stores the results in the output parameters.
  // Returns whether the import was successful.
  [[nodiscard]] virtual auto Import(std::filesystem::path const& src,
                                    std::vector<ResourceImportResult>& results) -> bool = 0;

  // Specifies whether this importer imports resources as-is or if it produces a custom binary.
  [[nodiscard]] virtual auto IsNativeImporter() const noexcept -> bool;

  ResourceImporter() = default;
  ResourceImporter(ResourceImporter const& other) = default;
  ResourceImporter(ResourceImporter&& other) noexcept = default;

  virtual ~ResourceImporter() = default;

  auto operator=(ResourceImporter const& other) -> ResourceImporter& = default;
  auto operator=(ResourceImporter&& other) noexcept -> ResourceImporter& = default;
};
}
