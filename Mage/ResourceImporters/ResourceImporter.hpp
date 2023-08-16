#pragma once

#include "../Core.hpp"
#include "../Reflection.hpp"
#include "../Resources/Resource.hpp"

#include <bit>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>


namespace sorcery {
class ResourceImporter {
  RTTR_ENABLE()

public:
  virtual auto GetSupportedFileExtensions(std::vector<std::string>& out) -> void = 0;
  [[nodiscard]] virtual auto Import(std::filesystem::path const& src, std::vector<std::byte>& bytes) -> bool = 0;
  [[nodiscard]] virtual auto GetImportedType(std::filesystem::path const& resPathAbs) noexcept -> rttr::type = 0;

  ResourceImporter() = default;
  ResourceImporter(ResourceImporter const& other) = default;
  ResourceImporter(ResourceImporter&& other) noexcept = default;

  virtual ~ResourceImporter() = default;

  auto operator=(ResourceImporter const& other) -> ResourceImporter& = default;
  auto operator=(ResourceImporter&& other) noexcept -> ResourceImporter& = default;
};
}
