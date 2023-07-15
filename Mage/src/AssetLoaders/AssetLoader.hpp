#pragma once

#include <filesystem>
#include <memory>
#include <span>

#include "Resource.hpp"


namespace sorcery::mage {
class AssetLoader {
public:
  AssetLoader() = default;
  AssetLoader(AssetLoader const& other) = default;
  AssetLoader(AssetLoader&& other) noexcept = default;

  auto operator=(AssetLoader const& other) -> AssetLoader& = default;
  auto operator=(AssetLoader&& other) noexcept -> AssetLoader& = default;

  virtual ~AssetLoader() = default;

  [[nodiscard]] virtual auto GetSupportedExtensions() const -> std::span<std::string const> = 0;
  [[nodiscard]] virtual auto Load(std::filesystem::path const& src, std::filesystem::path const& cache) -> std::unique_ptr<Resource> = 0;
  [[nodiscard]] virtual auto GetPrecedence() const noexcept -> int = 0;
};
}
