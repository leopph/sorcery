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


AssetStorage const& Context::GetResources() const noexcept {
  return mResources;
}


AssetStorage& Context::GetResources() noexcept {
  return mResources;
}


Scene const* Context::GetScene() const noexcept {
  return mScene;
}


Scene* Context::GetScene() noexcept {
  return mScene;
}


auto Context::OpenScene(Scene& scene) -> void {
  scene.Load(mFactoryManager);

  if (mScene) {
    mScene->Clear();
  }

  mScene = &scene;
}


ObjectWrapperManager const& Context::GetFactoryManager() const noexcept {
  return mFactoryManager;
}


ObjectWrapperManager& Context::GetFactoryManager() noexcept {
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
  mSelectedObject = nullptr;
  mScene = nullptr;
  mResources.Clear();
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

    mResources.RegisterAsset(std::unique_ptr<Object>{ asset }, info.assetPath);
  }
}


bool Context::IsEditorBusy() const noexcept {
  return mBusy;
}


auto Context::CreateMetaFileForRegisteredAsset(Object const& asset) const -> void {
  if (auto const assetPath{ mResources.TryGetPathFor(&asset) }; !assetPath.empty()) {
    std::ofstream{ std::filesystem::path{ assetPath } += ASSET_FILE_EXT } << GenerateAssetMetaFileContents(asset, mFactoryManager);
  }
}


auto Context::SaveRegisteredNativeAsset(NativeAsset const& asset) const -> void {
  if (auto const dst{ mResources.TryGetPathFor(&asset) }; !dst.empty()) {
    std::vector<std::uint8_t> static outSerializedBytes;
    outSerializedBytes.clear();

    asset.Serialize(outSerializedBytes);

    if (std::ofstream out{ dst, std::ios::out | std::ios::binary }; out.is_open()) {
      std::ranges::copy(outSerializedBytes, std::ostreambuf_iterator{ out });
    }
  }
}


auto Context::OnEnterBusyExecution() -> BusyExecutionContext {
  bool isBusy{ false };
  while (!mBusy.compare_exchange_weak(isBusy, true)) {}

  BusyExecutionContext const ret{
    .imGuiConfigFlagsBackup = mImGuiIo.ConfigFlags,
    .monoThread = gManagedRuntime.AttachToThisThread()
  };

  mImGuiIo.ConfigFlags |= ImGuiConfigFlags_NoMouse;
  mImGuiIo.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;

  return ret;
}


auto Context::OnFinishBusyExecution(BusyExecutionContext const& busyExecutionContext) -> void {
  mImGuiIo.ConfigFlags = busyExecutionContext.imGuiConfigFlagsBackup;
  gManagedRuntime.DetachFromThisThread(busyExecutionContext.monoThread);
  mBusy = false;
}
}
