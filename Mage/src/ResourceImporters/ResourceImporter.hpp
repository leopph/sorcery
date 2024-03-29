#pragma once

#include "../Core.hpp"
#include "../Reflection.hpp"
#include "../ExternalResource.hpp"

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>


namespace sorcery {
class ResourceImporter {
  RTTR_ENABLE()

public:
  virtual auto GetSupportedFileExtensions(std::pmr::vector<std::string>& out) -> void = 0;
  [[nodiscard]] virtual auto Import(std::filesystem::path const& src, std::vector<std::byte>& bytes, ExternalResourceCategory& categ) -> bool = 0;
  [[nodiscard]] virtual auto GetImportedType(std::filesystem::path const& resPathAbs) noexcept -> rttr::type = 0;
  [[nodiscard]] virtual auto IsNativeImporter() const noexcept -> bool;

  ResourceImporter() = default;
  ResourceImporter(ResourceImporter const& other) = default;
  ResourceImporter(ResourceImporter&& other) noexcept = default;

  virtual ~ResourceImporter() = default;

  auto operator=(ResourceImporter const& other) -> ResourceImporter& = default;
  auto operator=(ResourceImporter&& other) noexcept -> ResourceImporter& = default;
};
}
