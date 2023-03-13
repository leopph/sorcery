#pragma once

#include <unordered_map>
#include <filesystem>
#include <memory>

#include "Object.hpp"

namespace leopph::editor {
class AssetStorage {
	std::vector<std::unique_ptr<Object>> mAssets;
	std::unordered_map<std::filesystem::path, Object*> mPathToAsset;
	std::unordered_map<Object const*, std::filesystem::path> mAssetToPath;

public:
	auto RegisterAsset(std::unique_ptr<Object> asset, std::filesystem::path const& srcPath) -> void;

	[[nodiscard]] auto GetPathFor(Object const* asset) const -> std::filesystem::path const&;
	[[nodiscard]] auto TryGetPathFor(Object const* asset) const -> std::filesystem::path;
	[[nodiscard]] auto GetAssetAt(std::filesystem::path const& srcPath) const -> Object*;
	[[nodiscard]] auto TryGetAssetAt(std::filesystem::path const& srcPath) const -> Object*;

	auto Clear() -> void;

	[[nodiscard]] auto begin() const noexcept -> decltype(auto) {
		return std::begin(mPathToAsset);
	}

	[[nodiscard]] auto end() const noexcept -> decltype(auto) {
		return std::end(mPathToAsset);
	}

	[[nodiscard]] auto cbegin() const noexcept -> decltype(auto) {
		return std::cbegin(mPathToAsset);
	}

	[[nodiscard]] auto cend() const noexcept -> decltype(auto) {
		return std::cend(mPathToAsset);
	}
};
}
