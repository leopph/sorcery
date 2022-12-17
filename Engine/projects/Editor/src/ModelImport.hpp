#pragma once

#include "AssetManagement.hpp"

#include <StaticModelData.hpp>

#include <assimp/Importer.hpp>


namespace leopph::editor {
	class ModelAssetLoader : public AssetLoader {
	private:
		Assimp::Importer mImporter;

	public:
		auto CanLoad(std::filesystem::path const& assetPath) const -> bool override;
		auto Load(std::filesystem::path const& assetPath) -> std::shared_ptr<Asset> override;
	};


	class ModelAsset : public Asset {
	private:
		StaticModelData mModelData;

	public:
		explicit ModelAsset(std::filesystem::path assetPath, StaticModelData const& modelData);
		[[nodiscard]] auto GetModelData() const->StaticModelData const&;
		[[nodiscard]] auto GenerateMetaFile() const->std::string override;
	};
}