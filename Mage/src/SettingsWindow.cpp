#include "SettingsWindow.hpp"

#include "MemoryAllocation.hpp"
#include "Timing.hpp"
#include "Renderer.hpp"

#include <imgui.h>

#include <limits>


namespace sorcery::mage {
SettingsWindow::SettingsWindow(Application& app) :
  mIsOpen{true},
  mApp{std::addressof(app)} {}


auto SettingsWindow::Draw() -> void {
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

  ImGui::Text("Shadow Distance");
  ImGui::SameLine();

  float shadowDistance{gRenderer.GetShadowDistance()};
  if (ImGui::InputFloat("##shadowDistanceInput", &shadowDistance, 0, 0, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue)) {
    gRenderer.SetShadowDistance(shadowDistance);
  }

  ImGui::Text("Shadow Filtering Mode");
  ImGui::SameLine();

  auto constexpr shadowFilteringModeNames{
    [] {
      std::array<char const*, 6> ret{};
      ret[static_cast<int>(ShadowFilteringMode::None)] = "No Filtering";
      ret[static_cast<int>(ShadowFilteringMode::HardwarePCF)] = "PCF 2x2 (hardware)";
      ret[static_cast<int>(ShadowFilteringMode::PCF3x3)] = "PCF 3x3 (4 taps)";
      ret[static_cast<int>(ShadowFilteringMode::PCFTent3x3)] = "PCF Tent 3x3 (4 taps)";
      ret[static_cast<int>(ShadowFilteringMode::PCFTent5x5)] = "PCF Tent 5x5 (9 taps)";
      ret[static_cast<int>(ShadowFilteringMode::PCSS)] = "PCSS";
      return ret;
    }()
  };
  int currentShadowFilteringModeIdx{static_cast<int>(gRenderer.GetShadowFilteringMode())};
  if (ImGui::Combo("##ShadowFilteringModeCombo", &currentShadowFilteringModeIdx, shadowFilteringModeNames.data(), static_cast<int>(std::ssize(shadowFilteringModeNames)))) {
    gRenderer.SetShadowFilteringMode(static_cast<ShadowFilteringMode>(currentShadowFilteringModeIdx));
  }

  ImGui::Text("Visualize Shadow Cascades");
  ImGui::SameLine();

  bool visualizeShadowCascades{gRenderer.IsVisualizingShadowCascades()};
  if (ImGui::Checkbox("##VisualizeShadowCascadesCheckbox", &visualizeShadowCascades)) {
    gRenderer.VisualizeShadowCascades(visualizeShadowCascades);
  }

  ImGui::Text("Shadow Cascade Count");
  ImGui::SameLine();

  int cascadeCount{gRenderer.GetShadowCascadeCount()};
  if (ImGui::SliderInt("##cascadeCountInput", &cascadeCount, 1, gRenderer.GetMaxShadowCascadeCount(), "%d", ImGuiSliderFlags_NoInput)) {
    gRenderer.SetShadowCascadeCount(cascadeCount);
  }

  auto const cascadeSplits{gRenderer.GetNormalizedShadowCascadeSplits()};
  auto const splitCount{std::ssize(cascadeSplits)};

  for (int i = 0; i < splitCount; i++) {
    ImGui::Text("Split %d (percent)", i + 1);
    ImGui::SameLine();

    float cascadeSplit{cascadeSplits[i] * 100.0f};

    if (ImGui::SliderFloat(std::format("##cascadeSplit {} input", i).data(), &cascadeSplit, 0, 100, "%.3f", ImGuiSliderFlags_NoInput)) {
      gRenderer.SetNormalizedShadowCascadeSplit(i, cascadeSplit / 100.0f);
    }
  }

  ImGui::End();
}


auto SettingsWindow::SetOpen(bool const isOpen) noexcept -> void {
  mIsOpen = isOpen;
}
}
