#include "SettingsWindow.hpp"

#include "MemoryAllocation.hpp"
#include "Timing.hpp"
#include "Renderer.hpp"

#include <imgui.h>

#include <limits>


namespace sorcery::mage {
SettingsWindow::SettingsWindow(Application& app, StandaloneCamera& sceneViewCam) :
  mIsOpen{true},
  mApp{std::addressof(app)},
  mSceneViewCam{std::addressof(sceneViewCam)} {}


auto SettingsWindow::Draw() -> void {
  ImGui::SetNextWindowSizeConstraints(ImVec2{200, 200}, ImVec2{
    std::numeric_limits<float>::max(), std::numeric_limits<float>::max()
  });

  if (std::pmr::string windowName{&GetTmpMemRes()}; !ImGui::Begin(windowName.append(TITLE).append("##Window").c_str(),
    &mIsOpen)) {
    ImGui::End();
    return;
  }

  if (ImGui::TreeNode("Appearance")) {
    if (auto darkMode{mApp->IsGuiDarkMode()}; ImGui::Checkbox("Dark Mode", &darkMode)) {
      mApp->SetGuiDarkMode(darkMode);
    }
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Performance")) {
    bool isFrameRateLimited{timing::GetTargetFrameRate() != -1};

    if (ImGui::Checkbox("Frame Rate Limit", &isFrameRateLimited)) {
      timing::SetTargetFrameRate(isFrameRateLimited ? DEFAULT_TARGET_FRAME_RATE : -1);
    }

    if (isFrameRateLimited) {
      if (int targetFrameRate{timing::GetTargetFrameRate()}; ImGui::DragInt("Target Frame Rate", &targetFrameRate, 1, 30,
        std::numeric_limits<int>::max(), "%d", ImGuiSliderFlags_AlwaysClamp)) {
        timing::SetTargetFrameRate(targetFrameRate);
      }
    }

    if (int inFlightFrameCount{gRenderer.GetInFlightFrameCount()}; ImGui::SliderInt("In-Flight Frame Count",
      &inFlightFrameCount, Renderer::MIN_IN_FLIGHT_FRAME_COUNT, Renderer::MAX_IN_FLIGHT_FRAME_COUNT, "%d",
      ImGuiSliderFlags_AlwaysClamp)) {
      gRenderer.SetInFlightFrameCount(inFlightFrameCount);
    }

    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Rendering")) {
    if (auto gamma{gRenderer.GetGamma()}; ImGui::DragFloat("Gamma", &gamma, 0.01f, 0.01f, 5.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
      gRenderer.SetGamma(gamma);
    }

    constexpr char const* msaaComboLabels[]{"Off", "2x", "4x", "8x"};

    if (auto const msaaModeIdx{static_cast<int>(std::log2(static_cast<int>(gRenderer.GetMultisamplingMode())))};
      ImGui::BeginCombo("MSAA", msaaComboLabels[msaaModeIdx])) {
      for (auto i{0}; i < 4; i++) {
        if (ImGui::Selectable(msaaComboLabels[i], msaaModeIdx == i)) {
          gRenderer.SetMultisamplingMode(static_cast<Renderer::MultisamplingMode>(static_cast<int>(std::pow(2, i))));
        }
      }

      ImGui::EndCombo();
    }

    if (auto depthPrePassEnabled{gRenderer.IsDepthNormalPrePassEnabled()}; ImGui::Checkbox("Depth-Normal Pre-Pass", &depthPrePassEnabled)) {
      gRenderer.SetDepthNormalPrePassEnabled(depthPrePassEnabled);
    }

    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Lighting")) {
    if (auto color{gRenderer.GetAmbientLightColor()}; ImGui::ColorEdit3("Ambient Light Color", color.GetData())) {
      gRenderer.SetAmbientLightColor(color);
    }

    if (auto ssaoEnabled{gRenderer.IsSsaoEnabled()}; ImGui::Checkbox("SSAO", &ssaoEnabled)) {
      gRenderer.SetSsaoEnabled(ssaoEnabled);
    }

    ImGui::BeginDisabled(!gRenderer.IsSsaoEnabled());
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::NewLine();

    auto ssaoParams{gRenderer.GetSsaoParams()};

    if (ImGui::DragFloat("Radius", &ssaoParams.radius, 0.01f)) {
      gRenderer.SetSsaoParams(ssaoParams);
    }
    if (ImGui::DragFloat("Bias", &ssaoParams.bias, 0.01f)) {
      gRenderer.SetSsaoParams(ssaoParams);
    }
    if (ImGui::DragFloat("Power", &ssaoParams.power, 0.01f)) {
      gRenderer.SetSsaoParams(ssaoParams);
    }
    if (ImGui::DragInt("Sample Count", &ssaoParams.sampleCount, 1, 0, std::numeric_limits<int>::max())) {
      gRenderer.SetSsaoParams(ssaoParams);
    }

    ImGui::EndGroup();
    ImGui::EndDisabled();
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Shadows")) {
    float shadowDistance{gRenderer.GetShadowDistance()};
    if (ImGui::DragFloat("Shadow Distance", &shadowDistance, 1, 0, std::numeric_limits<float>::max(), "%.0f",
      ImGuiSliderFlags_AlwaysClamp)) {
      gRenderer.SetShadowDistance(shadowDistance);
    }

    auto constexpr shadowFilteringModeNames{
      [] {
        std::array<char const*, 6> ret{};
        ret[static_cast<int>(Renderer::ShadowFilteringMode::None)] = "No Filtering";
        ret[static_cast<int>(Renderer::ShadowFilteringMode::HardwarePCF)] = "PCF 2x2 (hardware)";
        ret[static_cast<int>(Renderer::ShadowFilteringMode::PCF3x3)] = "PCF 3x3 (4 taps)";
        ret[static_cast<int>(Renderer::ShadowFilteringMode::PCFTent3x3)] = "PCF Tent 3x3 (4 taps)";
        ret[static_cast<int>(Renderer::ShadowFilteringMode::PCFTent5x5)] = "PCF Tent 5x5 (9 taps)";
        ret[static_cast<int>(Renderer::ShadowFilteringMode::PCSS)] = "PCSS (Not yet implemented)";
        return ret;
      }()
    };

    if (int currentShadowFilteringModeIdx{static_cast<int>(gRenderer.GetShadowFilteringMode())};
      ImGui::Combo("Shadow Filtering Mode", &currentShadowFilteringModeIdx, shadowFilteringModeNames.data(), static_cast<int>(std::ssize(shadowFilteringModeNames)))) {
      gRenderer.SetShadowFilteringMode(static_cast<Renderer::ShadowFilteringMode>(currentShadowFilteringModeIdx));
    }

    if (int cascadeCount{gRenderer.GetShadowCascadeCount()}; ImGui::SliderInt("Shadow Cascade Count", &cascadeCount, 1,
      Renderer::GetMaxShadowCascadeCount(), "%d", ImGuiSliderFlags_NoInput)) {
      gRenderer.SetShadowCascadeCount(cascadeCount);
    }

    auto const cascadeSplits{gRenderer.GetNormalizedShadowCascadeSplits()};
    auto const splitCount{std::ssize(cascadeSplits)};

    for (int i = 0; i < splitCount; i++) {
      if (float cascadeSplit{cascadeSplits[i] * 100.0f}; ImGui::SliderFloat(
        std::format("Split {} (percent)", i + 1).data(), &cascadeSplit, 0, 100, "%.3f", ImGuiSliderFlags_NoInput)) {
        gRenderer.SetNormalizedShadowCascadeSplit(i, cascadeSplit / 100.0f);
      }
    }

    if (bool visualizeShadowCascades{gRenderer.IsVisualizingShadowCascades()}; ImGui::Checkbox(
      "Visualize Shadow Cascades", &visualizeShadowCascades)) {
      gRenderer.VisualizeShadowCascades(visualizeShadowCascades);
    }
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Scene View")) {
    ImGui::Text("%s", "Camera");

    auto constexpr clipPlaneMin{0.1f};
    auto constexpr clipPlaneMax{10'000.0f};
    auto constexpr clipPlaneSliderSpeed{0.1f};
    auto constexpr clipPlaneSliderFormat{"%.1f"};
    auto constexpr clipPlaneSliderFlags{ImGuiSliderFlags_AlwaysClamp};

    if (auto nearPlane{mSceneViewCam->GetNearClipPlane()}; ImGui::DragFloat("Near Clip Plane", &nearPlane,
      clipPlaneSliderSpeed, clipPlaneMin, clipPlaneMax, clipPlaneSliderFormat,
      clipPlaneSliderFlags)) {
      mSceneViewCam->SetNearClipPlane(nearPlane);
      mSceneViewCam->SetFarClipPlane(std::max(nearPlane, mSceneViewCam->GetFarClipPlane()));
    }

    if (auto farPlane{mSceneViewCam->GetFarClipPlane()}; ImGui::DragFloat("Far Clip Plane", &farPlane,
      clipPlaneSliderSpeed, clipPlaneMin, clipPlaneMax, clipPlaneSliderFormat,
      clipPlaneSliderFlags)) {
      mSceneViewCam->SetFarClipPlane(farPlane);
      mSceneViewCam->SetNearClipPlane(std::min(farPlane, mSceneViewCam->GetNearClipPlane()));
    }

    ImGui::DragFloat("Speed", &mSceneViewCam->speed, 0.1f, 0.1f, 10.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);

    if (auto fov{mSceneViewCam->GetVerticalPerspectiveFov()}; ImGui::SliderFloat("FOV", &fov, 5, 120, "%.0f", ImGuiSliderFlags_AlwaysClamp)) {
      mSceneViewCam->SetVerticalPerspectiveFov(fov);
    }

    ImGui::TreePop();
  }

  ImGui::End();
}


auto SettingsWindow::SetOpen(bool const isOpen) noexcept -> void {
  mIsOpen = isOpen;
}
}
