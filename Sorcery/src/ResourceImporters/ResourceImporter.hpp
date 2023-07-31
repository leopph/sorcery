#pragma once

#include "../Core.hpp"
#include "../Reflection.hpp"
#include "../Resources/Resource.hpp"

#include <filesystem>
#include <string>
#include <vector>


namespace sorcery {
class ResourceImporter {
  RTTR_ENABLE()

public:
  virtual auto GetSupportedFileExtensions(std::vector<std::string>& out) -> void = 0;
  [[nodiscard]] virtual auto Import(std::filesystem::path const& src) -> ObserverPtr<Resource> = 0;
  [[nodiscard]] virtual auto GetPrecedence() const noexcept -> int = 0;

  ResourceImporter() = default;
  ResourceImporter(ResourceImporter const& other) = delete;
  ResourceImporter(ResourceImporter&& other) = delete;

  virtual ~ResourceImporter() = default;

  auto operator=(ResourceImporter const& other) -> void = delete;
  auto operator=(ResourceImporter&& other) -> void = delete;
};
}
