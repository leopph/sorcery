#pragma once

#include <Object.hpp>

#include <filesystem>
#include <memory>
#include <vector>


namespace leopph::editor {
	enum class AssetType {
		Image, Model
	};

	struct AssetFileDescriptor {
		std::filesystem::path path;
		AssetType type;
	};

	auto DiscoverAssetsInFolder(std::filesystem::path const& workingDir, std::vector<AssetFileDescriptor>& out) -> std::vector<AssetFileDescriptor>&;

	class Asset : public Object {

	};

	class AssetImporter {
	public:
		virtual auto Import(std::filesystem::path const& assetPath) -> std::shared_ptr<Asset> = 0;
		virtual ~AssetImporter() = default;
	};

	struct Project {
		std::filesystem::path folder;
		std::vector<std::shared_ptr<Asset>> assets;
	};

	auto LoadProject(std::filesystem::path const& projectFolder) -> std::unique_ptr<Project>;
}