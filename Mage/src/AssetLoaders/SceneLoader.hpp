#pragma once

#include "AssetLoader.hpp"


namespace sorcery::mage {
class SceneLoader : public AssetLoader {
public:
  [[nodiscard]] auto GetSupportedExtensions() const -> std::span<std::string const> override;
  [[nodiscard]] auto Load(std::filesystem::path const& src, [[maybe_unused]] std::filesystem::path const& cache) -> std::unique_ptr<Object> override;
  [[nodiscard]] auto GetPrecedence() const noexcept -> int override;
};
}
