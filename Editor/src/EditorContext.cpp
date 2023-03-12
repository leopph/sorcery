#include "EditorContext.hpp"

#include <queue>
#include <fstream>

#include "Asset.hpp"

namespace leopph::editor {
Context::Context(ImGuiIO& imGuiIO) :
	mImGuiIo{ imGuiIO } { }

ImGuiIO const& Context::GetImGuiIo() const noexcept {
	return mImGuiIo;
}

ImGuiIO& Context::GetImGuiIo() noexcept {
	return mImGuiIo;
}

ResourceStorage const& Context::GetResources() const noexcept {
	return mResources;
}

ResourceStorage& Context::GetResources() noexcept {
	return mResources;
}

std::shared_ptr<Scene const> Context::GetScene() const noexcept {
	return mScene;
}

std::shared_ptr<Scene> Context::GetScene() noexcept {
	return mScene;
}

EditorObjectFactoryManager const& Context::GetFactoryManager() const noexcept {
	return mFactoryManager;
}

EditorObjectFactoryManager& Context::GetFactoryManager() noexcept {
	return mFactoryManager;
}

Object* Context::GetSelectedObject() const noexcept {
	return mSelectedObject;
}

void Context::SetSelectedObject(Object* const obj) noexcept {
	mSelectedObject = obj;
}

std::filesystem::path const& Context::GetProjectDirectoryAbsolute() const noexcept {
	return mProjDirAbs;
}

std::filesystem::path const& Context::GetAssetDirectoryAbsolute() const noexcept {
	return mAssetDirAbs;
}

std::filesystem::path const& Context::GetCacheDirectoryAbsolute() const noexcept {
	return mCacheDirAbs;
}

auto Context::OpenProject(std::filesystem::path const& targetPath) -> void {
	mScene = std::make_shared<Scene>();
	mResources.clear();
	mProjDirAbs = absolute(targetPath);
	mAssetDirAbs = absolute(mProjDirAbs / ASSET_DIR_REL);
	mCacheDirAbs = absolute(mProjDirAbs / CACHE_DIR_REL);

	if (!exists(mAssetDirAbs)) {
		create_directory(mAssetDirAbs);
	}

	if (!exists(mCacheDirAbs)) {
		create_directory(mCacheDirAbs);
	}

	struct ImportInfo {
		std::filesystem::path assetPath;
		AssetMetaInfo metaInfo;
	};

	std::priority_queue<ImportInfo, std::vector<ImportInfo>, decltype([](ImportInfo const& lhs, ImportInfo const& rhs) {
		return lhs.metaInfo.importPrecedence > rhs.metaInfo.importPrecedence;
	})> queue;

	for (auto const& entry : std::filesystem::recursive_directory_iterator{ mAssetDirAbs }) {
		if (entry.path().extension() == ASSET_FILE_EXT) {
			std::ifstream in{ entry.path(), std::ios::binary };
			std::stringstream buffer;
			buffer << in.rdbuf();
			auto const assetPath{ std::filesystem::path{ entry.path() }.replace_extension() };
			auto const meta{ ReadAssetMetaFileContents(buffer.str()) };
			queue.emplace(assetPath, meta);
		}
	}

	while (!queue.empty()) {
		ImportInfo info{ queue.top() };
		queue.pop();

		auto& factory{ mFactoryManager.GetFor(info.metaInfo.type) };

		Importer::InputImportInfo const inputImportInfo{
			.src = info.assetPath,
			.guid = info.metaInfo.guid
		};

		auto const asset{ factory.GetImporter().Import(inputImportInfo, mCacheDirAbs) };
		asset->SetName(info.assetPath.stem().string());
		asset->SetGuid(info.metaInfo.guid);

		mResources[info.assetPath] = std::shared_ptr<Object>{ asset };
	}
}

bool Context::IsEditorBusy() const noexcept {
	return mBusy;
}
}