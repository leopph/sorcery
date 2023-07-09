#include "EditorContext.hpp"

#include "Systems.hpp"

#include <queue>
#include <fstream>

#include "Asset.hpp"


namespace sorcery::mage {
auto Context::OnWindowFocusGain(Context* const self) -> void {
  if (!self->mProjDirAbs.empty()) {
    self->mResourceManager.Refresh();
  }
}


auto Context::HandleBackgroundThreadException(std::exception const& ex) -> void {
  DisplayError(ex.what());
}


auto Context::HandleUnknownBackgroundThreadException() -> void {
  DisplayError("Unknown error.");
}


Context::Context(ImGuiIO& imGuiIO) :
  mImGuiIo{ imGuiIO } {
  gWindow.OnWindowFocusGain.add_handler(this, &OnWindowFocusGain);
}


Context::~Context() {
  gWindow.OnWindowFocusGain.remove_handler(this, &OnWindowFocusGain);
}


auto Context::GetImGuiIo() const noexcept -> ImGuiIO const& {
  return mImGuiIo;
}


auto Context::GetImGuiIo() noexcept -> ImGuiIO& {
  return mImGuiIo;
}


auto Context::GetResources() const noexcept -> ResourceManager const& {
  return mResourceManager;
}


auto Context::GetResources() noexcept -> ResourceManager& {
  return mResourceManager;
}


auto Context::GetScene() const noexcept -> Scene const* {
  return mScene;
}


auto Context::GetScene() noexcept -> Scene* {
  return mScene;
}


auto Context::OpenScene(Scene& scene) -> void {
  scene.Load(*mWrapperManager);

  if (mScene) {
    mScene->Clear();
  }

  mScene = &scene;
}


auto Context::GetFactoryManager() const noexcept -> ObjectWrapperManager const& {
  return *mWrapperManager;
}


auto Context::GetFactoryManager() noexcept -> ObjectWrapperManager& {
  return *mWrapperManager;
}


auto Context::GetSelectedObject() const noexcept -> Object* {
  return mSelectedObject;
}


auto Context::SetSelectedObject(Object* const obj) noexcept -> void {
  mSelectedObject = obj;
}


auto Context::GetProjectDirectoryAbsolute() const noexcept -> std::filesystem::path const& {
  return mProjDirAbs;
}


auto Context::GetAssetDirectoryAbsolute() const noexcept -> std::filesystem::path const& {
  return mAssetDirAbs;
}


auto Context::GetCacheDirectoryAbsolute() const noexcept -> std::filesystem::path const& {
  return mCacheDirAbs;
}


auto Context::GetAssetDirectoryProjectRootRelative() noexcept -> std::filesystem::path const& {
  return ASSET_DIR_REL;
}


auto Context::GetCacheDirectoryProjectRootRelative() noexcept -> std::filesystem::path const& {
  return CACHE_DIR_REL;
}


auto Context::OpenProject(std::filesystem::path const& targetPath) -> void {
  mSelectedObject = nullptr;
  mScene = nullptr;
  mResourceManager.Clear();
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

    auto& factory{ mWrapperManager->GetFor(info.metaInfo.type) };

    auto asset{ factory.GetLoader().Load(info.assetPath, mCacheDirAbs) };
    asset->SetName(info.assetPath.stem().string());
    asset->SetGuid(info.metaInfo.guid);

    mResourceManager.RegisterAsset(std::move(asset), info.assetPath);
  }
}


auto Context::IsEditorBusy() const noexcept -> bool {
  return mBusy;
}


auto Context::CreateMetaFileForRegisteredAsset(Object const& asset) const -> void {
  if (auto const assetPath{ mResourceManager.TryGetPathFor(&asset) }; !assetPath.empty()) {
    std::ofstream{ std::filesystem::path{ assetPath } += ASSET_FILE_EXT } << GenerateAssetMetaFileContents(asset, *mWrapperManager);
  }
}


auto Context::SaveRegisteredNativeAsset(NativeAsset const& asset) const -> void {
  if (auto const dst{ mResourceManager.TryGetPathFor(&asset) }; !dst.empty()) {
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
    .imGuiConfigFlagsBackup = mImGuiIo.ConfigFlags
  };

  mImGuiIo.ConfigFlags |= ImGuiConfigFlags_NoMouse;
  mImGuiIo.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;

  return ret;
}


auto Context::OnFinishBusyExecution(BusyExecutionContext const& busyExecutionContext) -> void {
  mImGuiIo.ConfigFlags = busyExecutionContext.imGuiConfigFlagsBackup;
  mBusy = false;
}
}
