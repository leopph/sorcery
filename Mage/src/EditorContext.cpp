#include "EditorContext.hpp"

#include "Platform.hpp"


namespace sorcery::mage {
auto Context::OnWindowFocusGain(Context* const self) -> void {
  if (!self->mProjDirAbs.empty()) {
    self->mResourceDB.Refresh();
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


auto Context::GetResourceDatabase() const noexcept -> ResourceDB const& {
  return mResourceDB;
}


auto Context::GetResourceDatabase() noexcept -> ResourceDB& {
  return mResourceDB;
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


auto Context::OpenProject(std::filesystem::path const& targetPath) -> void {
  mSelectedObject = nullptr;
  mScene = nullptr;
  mProjDirAbs = absolute(targetPath);
  mResourceDB.ChangeProjectDir(mProjDirAbs);
}


auto Context::IsEditorBusy() const noexcept -> bool {
  return mBusy;
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
