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
	private:
		std::filesystem::path mPath;

	public:
		explicit Asset(std::filesystem::path assetPath);
		[[nodiscard]] auto GetPath() const->std::filesystem::path;
		[[nodiscard]] virtual auto GenerateMetaFile() const->std::string = 0;
	};

	class AssetLoader {
	public:
		virtual auto CanLoad(std::filesystem::path const& assetPath) const -> bool = 0;
		virtual auto Load(std::filesystem::path const& assetPath) -> std::shared_ptr<Asset> = 0;
		virtual ~AssetLoader() = default;
	};

	struct Project {
		std::filesystem::path folder;
		std::vector<std::shared_ptr<Asset>> assets;
	};

	auto LoadProject(std::filesystem::path const& projectFolder) -> std::unique_ptr<Project>;

	[[nodiscard]] auto LoadAsset(std::filesystem::path const& assetPath) -> std::shared_ptr<Asset>;
	[[nodiscard]] auto LoadAsset(AssetFileDescriptor const& desc) -> std::shared_ptr<Asset>;
	[[nodiscard]] auto ImportAsset(std::filesystem::path const& srcPath, std::filesystem::path const& dstDirPath) -> std::shared_ptr<Asset>;
}