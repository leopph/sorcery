#include "AssetStorage.hpp"

namespace leopph::editor {
auto AssetStorage::RegisterAsset(std::unique_ptr<Object> asset, std::filesystem::path const& srcPath) -> void {
	mAssetToPath[asset.get()] = srcPath;
	mPathToAsset[srcPath] = asset.get();
	mAssets.emplace_back(std::move(asset));
}

auto AssetStorage::GetPathFor(Object const* asset) const -> std::filesystem::path const& {
	return mAssetToPath.at(asset);
}

auto AssetStorage::TryGetPathFor(Object const* asset) const -> std::filesystem::path {
	if (auto const it{ mAssetToPath.find(asset) }; it != std::end(mAssetToPath)) {
		return it->second;
	}
	return {};
}

auto AssetStorage::GetAssetAt(std::filesystem::path const& srcPath) const -> Object* {
	return mPathToAsset.at(srcPath);
}

auto AssetStorage::TryGetAssetAt(std::filesystem::path const& srcPath) const -> Object* {
	if (auto const it{ mPathToAsset.find(srcPath) }; it != std::end(mPathToAsset)) {
		return it->second;
	}
	return nullptr;
}

auto AssetStorage::Clear() -> void {
	mAssetToPath.clear();
	mPathToAsset.clear();
	mAssets.clear();
}
}
