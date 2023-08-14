#include "EditorSettingsWindow.hpp"

#include "MemoryAllocation.hpp"
#include "Timing.hpp"
#include "Renderer.hpp"

#include <imgui.h>

#include <limits>


namespace sorcery::mage {
EditorSettingsWindow::EditorSettingsWindow(Application& app) :
  mIsOpen{false},
  mApp{std::addressof(app)} {}


auto EditorSettingsWindow::Draw() -> void {
  ImGui::SetNextWindowSizeConstraints(ImVec2{200, 200}, ImVec2{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});

  if (std::pmr::string windowName{&GetTmpMemRes()}; !ImGui::Begin(windowName.append(TITLE).append("##Window").c_str(), &mIsOpen)) {
    ImGui::End();
    return;
  }

  ImGui::Text("%s", "Frame Rate Limit");
  ImGui::SameLine();

  bool isFrameRateLimited{timing::GetTargetFrameRate() != -1};
  if (ImGui::Checkbox("##FrameRateLimitCheckbox", &isFrameRateLimited)) {
    timing::SetTargetFrameRate(isFrameRateLimited
                                 ? DEFAULT_TARGET_FRAME_RATE
                                 : -1);
  }

  if (isFrameRateLimited) {
    ImGui::Text("%s", "Target Frame Rate");
    ImGui::SameLine();

    int targetFrameRate{timing::GetTargetFrameRate()};
    if (ImGui::DragInt("##TargetFrameRateWidget", &targetFrameRate, 1, 30, std::numeric_limits<int>::max(), "%d", ImGuiSliderFlags_AlwaysClamp)) {
      timing::SetTargetFrameRate(targetFrameRate);
    }
  }

  ImGui::Text("%s", "Dark Mode");
  ImGui::SameLine();

  if (auto darkMode{mApp->IsGuiDarkMode()}; ImGui::Checkbox("##DarkModeCheckbox", &darkMode)) {
    mApp->SetGuiDarkMode(darkMode);
  }

  ImGui::Text("%s", "In-Flight Frame Count");
  ImGui::SameLine();

  int inFlightFrameCount{gRenderer.GetInFlightFrameCount()};
  if (ImGui::SliderInt("##InFlightFrameCountSlider", &inFlightFrameCount, Renderer::MIN_IN_FLIGHT_FRAME_COUNT, Renderer::MAX_IN_FLIGHT_FRAME_COUNT, "%d", ImGuiSliderFlags_AlwaysClamp)) {
    gRenderer.SetInFlightFrameCount(inFlightFrameCount);
  }

  ImGui::End();
}


auto EditorSettingsWindow::SetOpen(bool const isOpen) noexcept -> void {
  mIsOpen = isOpen;
}
}
