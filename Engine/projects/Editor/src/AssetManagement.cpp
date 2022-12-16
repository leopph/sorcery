#include "AssetManagement.hpp"

#include <filesystem>


namespace leopph::editor {
	namespace {
		auto constexpr METADATA_FILE_EXTENSION{ ".leopphasset" };
		[[nodiscard]] AssetType GetAssetType(std::filesystem::path const& metaPath) {
			return AssetType::Model;
		}
	}


	auto DiscoverAssetsInFolder(std::filesystem::path const& workingDir, std::vector<AssetFileDescriptor>& out) -> std::vector<AssetFileDescriptor>& {
		out.clear();
		for (auto const& childEntry : std::filesystem::recursive_directory_iterator{ workingDir }) {
			if (auto const childPath{ std::filesystem::absolute(childEntry.path()) }; childPath.extension() == METADATA_FILE_EXTENSION) {
				out.emplace_back(childPath, GetAssetType(childPath));
			}
		}
		return out;
	}


	auto LoadProject(std::filesystem::path const& projectFolder) -> std::unique_ptr<Project> {
		std::vector<AssetFileDescriptor> static assetDescs;
		DiscoverAssetsInFolder(projectFolder, assetDescs);
		return std::make_unique<Project>(projectFolder);
	}
}