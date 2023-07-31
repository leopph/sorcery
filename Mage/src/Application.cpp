#include "Application.hpp"

#include "Platform.hpp"
#include "GUI.hpp"


namespace sorcery::mage {
auto Application::OnWindowFocusGain(Application* const self) -> void {
  if (!self->mProjDirAbs.empty()) {
    self->mResourceDB.Refresh();
  }
}


auto Application::HandleBackgroundThreadException(std::exception const& ex) -> void {
  DisplayError(ex.what());
}


auto Application::HandleUnknownBackgroundThreadException() -> void {
  DisplayError("Unknown error.");
}


Application::Application(ImGuiIO& imGuiIO) :
  mImGuiIo{ imGuiIO } {
  gWindow.OnWindowFocusGain.add_handler(this, &OnWindowFocusGain);
  SetImGuiContext(*ImGui::GetCurrentContext());
}


Application::~Application() {
  gWindow.OnWindowFocusGain.remove_handler(this, &OnWindowFocusGain);
}


auto Application::GetImGuiIo() const noexcept -> ImGuiIO const& {
  return mImGuiIo;
}


auto Application::GetImGuiIo() noexcept -> ImGuiIO& {
  return mImGuiIo;
}


auto Application::GetResourceDatabase() const noexcept -> ResourceDB const& {
  return mResourceDB;
}


auto Application::GetResourceDatabase() noexcept -> ResourceDB& {
  return mResourceDB;
}


auto Application::GetScene() const noexcept -> Scene const* {
  return mScene;
}


auto Application::GetScene() noexcept -> Scene* {
  return mScene;
}


auto Application::OpenScene(Scene& scene) -> void {
  scene.Load();

  if (mScene) {
    mScene->Clear();
  }

  mScene = &scene;
}


auto Application::GetSelectedObject() const noexcept -> Object* {
  return mSelectedObject;
}


auto Application::SetSelectedObject(Object* const obj) noexcept -> void {
  mSelectedObject = obj;
}


auto Application::GetProjectDirectoryAbsolute() const noexcept -> std::filesystem::path const& {
  return mProjDirAbs;
}


auto Application::OpenProject(std::filesystem::path const& targetPath) -> void {
  mSelectedObject = nullptr;
  mScene = nullptr;
  mProjDirAbs = absolute(targetPath);
  mResourceDB.ChangeProjectDir(mProjDirAbs);
}


auto Application::IsEditorBusy() const noexcept -> bool {
  return mBusy;
}


auto Application::OnEnterBusyExecution() -> BusyExecutionContext {
  bool isBusy{ false };
  while (!mBusy.compare_exchange_weak(isBusy, true)) {}

  BusyExecutionContext const ret{
    .imGuiConfigFlagsBackup = mImGuiIo.ConfigFlags
  };

  mImGuiIo.ConfigFlags |= ImGuiConfigFlags_NoMouse;
  mImGuiIo.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;

  return ret;
}


auto Application::OnFinishBusyExecution(BusyExecutionContext const& busyExecutionContext) -> void {
  mImGuiIo.ConfigFlags = busyExecutionContext.imGuiConfigFlagsBackup;
  mBusy = false;
}
}
