#pragma once

#include "Core.hpp"
#include "ModelData.hpp"

#include <assimp/Importer.hpp>

#include <filesystem>
#include <memory>
#include <vector>


namespace leopph {
	[[nodiscard]] auto LoadRawModelAsset(std::filesystem::path const& src) -> std::unique_ptr<Assimp::Importer>;
	[[nodiscard]] auto ProcessRawModelAssetData(aiScene const& importedScene, std::filesystem::path const& src) -> ModelData;
	[[nodiscard]] auto GenerateLeopphAsset(ModelData const& processedData, std::vector<u8>& out) -> std::vector<u8>&;
	auto LoadLeopphModelAsset(std::filesystem::path const& src) -> void;
}
