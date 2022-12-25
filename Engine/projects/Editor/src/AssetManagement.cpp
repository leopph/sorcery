#include "AssetManagement.hpp"

#include <YamlInclude.hpp>

#include <filesystem>
#include <fstream>
#include <algorithm>


namespace leopph::editor {
	namespace {
		std::filesystem::path const METADATA_FILE_EXTENSION{ L".leopphasset" };
	}


	auto DiscoverAssetsInFolder(std::filesystem::path const& workingDir, std::vector<AssetFileDescriptor>& out) -> std::vector<AssetFileDescriptor>& {
		out.clear();
		for (auto const& childEntry : std::filesystem::recursive_directory_iterator{ workingDir }) {
			if (auto const childPath{ std::filesystem::absolute(childEntry.path()) }; childPath.extension() == METADATA_FILE_EXTENSION) {
				std::ifstream metaIn{ childPath };
				auto const metaContents{ YAML::Load(metaIn) };
				out.emplace_back(std::filesystem::path{ childPath }.replace_extension(), static_cast<AssetType>(metaContents["assetType"].as<int>()), Guid::Parse(metaContents["guid"].as<std::string>()));
			}
		}
		return out;
	}


	auto LoadProject(std::filesystem::path const& projectFolder) -> std::unique_ptr<Project> {
		std::vector<AssetFileDescriptor> static assetDescs;
		DiscoverAssetsInFolder(projectFolder, assetDescs);
		auto project{ std::make_unique<Project>(projectFolder) };
		for (auto const& desc : assetDescs) {
			if (auto const asset{ LoadExistingAsset(desc) }; asset) {
				project->assets.emplace_back(asset);
			}
		}
		return project;
	}

	auto LoadNewAsset(std::filesystem::path const& assetPath) -> std::shared_ptr<Asset> {
		std::shared_ptr<Asset> asset;
		
		/*if (ModelAssetLoader static modelAssetLoader; modelAssetLoader.CanLoad(assetPath)) {
			asset = modelAssetLoader.Load(assetPath);
		}

		if (asset) {
			asset->SetGuid(Guid::Generate());
		}*/

		return asset;
	}

	auto LoadExistingAsset(AssetFileDescriptor const& desc) -> std::shared_ptr<Asset> {
		std::shared_ptr<Asset> asset;

		/*switch (desc.type) {
			case AssetType::Model: {
				ModelAssetLoader static modelAssetLoader;
				asset = modelAssetLoader.Load(desc.absolutePath);
				break;
			}
		}*/

		if (asset) {
			asset->SetGuid(desc.guid);
		}

		return nullptr;
	}

	auto ImportAsset(std::filesystem::path const& srcPath, std::filesystem::path const& dstDirPath) -> std::shared_ptr<Asset> {
		auto const dstPath{ dstDirPath / srcPath.filename() };
		if (!std::filesystem::exists(srcPath)) {
			return nullptr;
		}
		if (std::filesystem::exists(dstPath)) {
			return nullptr;
		}
		if (!std::filesystem::equivalent(srcPath.parent_path(), dstDirPath)) {
			std::filesystem::copy(srcPath, dstPath);
		}
		auto const asset{ LoadNewAsset(dstPath) };
		if (asset) {
			auto const metaFilePath{ [&dstPath] {
				auto ret{dstPath};
				ret += METADATA_FILE_EXTENSION;
				return ret;
			}() };
			std::ofstream metaOut{ metaFilePath };
			metaOut << asset->GenerateMetaFile();
		}
		return asset;
	}


	Asset::Asset(std::filesystem::path assetPath) :
		mPath{ std::move(assetPath) } {}


	auto Asset::GetPath() const -> std::filesystem::path {
		return mPath;
	}
}