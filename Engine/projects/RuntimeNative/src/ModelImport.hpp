#pragma once

#include "Core.hpp"
#include "ModelData.hpp"

#include <assimp/Importer.hpp>

#include <bit>
#include <filesystem>
#include <memory>
#include <vector>


namespace leopph {
	[[nodiscard]] auto LoadRawModelAsset(std::filesystem::path const& src) -> std::unique_ptr<Assimp::Importer>;
	[[nodiscard]] auto ProcessRawModelAssetData(aiScene const& importedScene, std::filesystem::path const& src) -> ModelData;
	[[nodiscard]] auto GenerateModelLeopphAsset(ModelData const& modelData, std::vector<u8>& out, std::endian endianness) -> std::vector<u8>&;
	[[nodiscard]] auto LoadLeopphModelAsset(std::filesystem::path const& src) -> ModelData;
}
