#include "Application.hpp"

#include "Platform.hpp"
#include "GUI.hpp"
#include "NativeResourceImporter.hpp"

#include <nfd.h>


namespace sorcery::mage {
std::string_view const Application::WINDOW_TITLE_BASE{"Mage"};


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
  mImGuiIo{imGuiIO} {
  gWindow.SetTitle(std::string{WINDOW_TITLE_BASE});
  gWindow.OnWindowFocusGain.add_handler(this, &OnWindowFocusGain);
  SetImGuiContext(*ImGui::GetCurrentContext());
  SetGuiDarkMode(mIsInDarkMode);

  auto const font{imGuiIO.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\Segoeui.ttf)", 16)};
  imGuiIO.FontDefault = font;
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


auto Application::GetScene() const noexcept -> Scene& {
  assert(mScene);
  return *mScene;
}


auto Application::OpenScene(Scene& scene) -> void {
  if (std::addressof(scene) != mScene) {
    scene.Load();
    assert(mScene);
    mScene->Clear();
    mScene = std::addressof(scene);
  }
}


auto Application::SaveCurrentSceneToFile() -> void {
  assert(mScene);
  mScene->Save();
  if (mResourceDB.IsSavedResource(*mScene)) {
    mResourceDB.SaveResource(*mScene);
  } else {
    if (nfdchar_t* dst; NFD_SaveDialog(NativeResourceImporter::SCENE_FILE_EXT.substr(1).data(), mResourceDB.GetResourceDirectoryAbsolutePath().string().c_str(), &dst) == NFD_OKAY) {
      if (auto const dstResDirRel{relative(std::filesystem::path{dst}, mResourceDB.GetResourceDirectoryAbsolutePath()) += NativeResourceImporter::SCENE_FILE_EXT}; !dstResDirRel.empty()) {
        mResourceDB.CreateResource(*mScene, dstResDirRel);
      }
      std::free(dst);
    }
  }
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
  mScene = new Scene{};
  mProjDirAbs = absolute(targetPath);
  mResourceDB.ChangeProjectDir(mProjDirAbs);
  gWindow.SetTitle(std::string{WINDOW_TITLE_BASE} + " - " + targetPath.stem().string());
}


auto Application::IsEditorBusy() const noexcept -> bool {
  return mBusy;
}


auto Application::IsGuiDarkMode() const noexcept -> bool {
  return mIsInDarkMode;
}


auto Application::SetGuiDarkMode(bool const darkMode) noexcept -> void {
  auto& style{ImGui::GetStyle()};

  if (darkMode) {
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.48f, 0.48f, 0.48f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.68f, 0.68f, 0.69f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.77f, 0.77f, 0.78f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.48f, 0.48f, 0.48f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.22f, 0.22f, 0.22f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.33f, 0.33f, 0.33f, 0.97f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.68f, 0.68f, 0.68f, 1.00f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.62f, 0.62f, 0.62f, 1.00f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.53f, 0.46f, 0.62f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.46f);
  } else {
    ImGui::StyleColorsLight();
  }

  style.ChildBorderSize = 0;
  style.FrameBorderSize = 0;
  style.PopupBorderSize = 0;
  style.SeparatorTextBorderSize = 0;
  style.TabBorderSize = 0;
  style.WindowBorderSize = 0;
  style.ChildRounding = 4;
  style.FrameRounding = 4;
  style.GrabRounding = 4;
  style.PopupRounding = 4;
  style.ScrollbarRounding = 4;
  style.TabRounding = 4;
  style.WindowRounding = 4;
  mIsInDarkMode = darkMode;
}


auto Application::OnEnterBusyExecution() -> BusyExecutionContext {
  bool isBusy{false};
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
