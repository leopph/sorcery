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


Context::Context(ImGuiIO& imGuiIO) :
  mImGuiIo{ imGuiIO } {
  gWindow.OnWindowFocusGain.add_handler(this, &OnWindowFocusGain);
}


Context::~Context() {
  gWindow.OnWindowFocusGain.remove_handler(this, &OnWindowFocusGain);
}


ImGuiIO const& Context::GetImGuiIo() const noexcept {
  return mImGuiIo;
}


ImGuiIO& Context::GetImGuiIo() noexcept {
  return mImGuiIo;
}


ResourceManager const& Context::GetResources() const noexcept {
  return mResourceManager;
}


ResourceManager& Context::GetResources() noexcept {
  return mResourceManager;
}


Scene const* Context::GetScene() const noexcept {
  return mScene;
}


Scene* Context::GetScene() noexcept {
  return mScene;
}


auto Context::OpenScene(Scene& scene) -> void {
  scene.Load(*mWrapperManager);

  if (mScene) {
    mScene->Clear();
  }

  mScene = &scene;
}


ObjectWrapperManager const& Context::GetFactoryManager() const noexcept {
  return *mWrapperManager;
}


ObjectWrapperManager& Context::GetFactoryManager() noexcept {
  return *mWrapperManager;
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


bool Context::IsEditorBusy() const noexcept {
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
