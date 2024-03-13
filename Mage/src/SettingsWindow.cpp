#include "SettingsWindow.hpp"

#include "MemoryAllocation.hpp"
#include "Timing.hpp"
#include "..\..\Sorcery\src\rendering\scene_renderer.hpp"

#include <imgui.h>

#include <limits>


namespace sorcery::mage {
SettingsWindow::SettingsWindow(Application& app, StandaloneCamera& sceneViewCam) :
  mApp{std::addressof(app)},
  mSceneViewCam{std::addressof(sceneViewCam)} {}


auto SettingsWindow::Draw() -> void {
  ImGui::SetNextWindowSizeConstraints(ImVec2{200, 200}, ImVec2{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});

  if (std::pmr::string windowName{&GetTmpMemRes()}; !ImGui::Begin(windowName.append(TITLE).append("##Window").c_str())) {
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

    ImGui::BeginDisabled(!isFrameRateLimited);
    if (int targetFrameRate{timing::GetTargetFrameRate()}; ImGui::DragInt("Target Frame Rate", &targetFrameRate, 1, 30, std::numeric_limits<int>::max(), "%d", ImGuiSliderFlags_AlwaysClamp)) {
      timing::SetTargetFrameRate(targetFrameRate);
    }
    ImGui::EndDisabled();

    if (int inFlightFrameCount{gRenderer.GetInFlightFrameCount()}; ImGui::SliderInt("In-Flight Frame Count", &inFlightFrameCount, Renderer::MIN_IN_FLIGHT_FRAME_COUNT, Renderer::MAX_IN_FLIGHT_FRAME_COUNT, "%d", ImGuiSliderFlags_AlwaysClamp)) {
      gRenderer.SetInFlightFrameCount(inFlightFrameCount);
    }

    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Rendering")) {
    if (auto preciseColor{gRenderer.IsUsingPreciseColorFormat()}; ImGui::Checkbox("Use Precise Color Buffer Format", &preciseColor)) {
      gRenderer.SetUsePreciseColorFormat(preciseColor);
    }

    constexpr char const* msaaComboLabels[]{"Off", "2x", "4x", "8x"};

    if (auto const msaaModeIdx{static_cast<int>(std::log2(static_cast<int>(gRenderer.GetMultisamplingMode())))}; ImGui::BeginCombo("MSAA", msaaComboLabels[msaaModeIdx])) {
      for (auto i{0}; i < 4; i++) {
        if (ImGui::Selectable(msaaComboLabels[i], msaaModeIdx == i)) {
          gRenderer.SetMultisamplingMode(static_cast<Renderer::MultisamplingMode>(static_cast<int>(std::pow(2, i))));
        }
      }

      ImGui::EndCombo();
    }

    if (auto gamma{gRenderer.GetGamma()}; ImGui::DragFloat("Gamma", &gamma, 0.01f, 0.01f, 5.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
      gRenderer.SetGamma(gamma);
    }

    if (auto depthPrePassEnabled{gRenderer.IsDepthNormalPrePassEnabled()}; ImGui::Checkbox("Depth-Normal Pre-Pass", &depthPrePassEnabled)) {
      gRenderer.SetDepthNormalPrePassEnabled(depthPrePassEnabled);
    }

    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Lighting")) {
    if (auto color{Scene::GetActiveScene()->GetAmbientLightVector()}; ImGui::ColorEdit3("Ambient Light", color.GetData())) {
      Scene::GetActiveScene()->SetAmbientLightVector(color);
    }

    if (auto ssaoEnabled{gRenderer.IsSsaoEnabled()}; ImGui::Checkbox("Screen Space Ambient Occlusion", &ssaoEnabled)) {
      gRenderer.SetSsaoEnabled(ssaoEnabled);
    }

    ImGui::BeginDisabled(!gRenderer.IsSsaoEnabled());
    ImGui::Indent();

    auto ssaoParams{gRenderer.GetSsaoParams()};

    if (ImGui::DragFloat("Radius", &ssaoParams.radius, 0.01f, std::numeric_limits<float>::min(), std::numeric_limits<float>::max() / 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
      gRenderer.SetSsaoParams(ssaoParams);
    }
    if (ImGui::DragFloat("Bias", &ssaoParams.bias, 0.001f, 0, std::numeric_limits<float>::max() / 2.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
      gRenderer.SetSsaoParams(ssaoParams);
    }
    if (ImGui::DragFloat("Power", &ssaoParams.power, 0.01f, std::numeric_limits<float>::min(), std::numeric_limits<float>::max() / 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
      gRenderer.SetSsaoParams(ssaoParams);
    }
    if (ImGui::DragInt("Sample Count", &ssaoParams.sampleCount, 1, 0, std::numeric_limits<int>::max(), "%d", ImGuiSliderFlags_AlwaysClamp)) {
      gRenderer.SetSsaoParams(ssaoParams);
    }

    ImGui::Unindent();
    ImGui::EndDisabled();
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Shadows")) {
    float shadowDistance{gRenderer.GetShadowDistance()};
    if (ImGui::DragFloat("Shadow Distance", &shadowDistance, 1, 0, std::numeric_limits<float>::max(), "%.0f", ImGuiSliderFlags_AlwaysClamp)) {
      gRenderer.SetShadowDistance(shadowDistance);
    }

    auto constexpr shadowFilteringModeNames{
      [] {
        std::array<char const*, 5> ret{};
        ret[static_cast<int>(Renderer::ShadowFilteringMode::None)] = "No Filtering";
        ret[static_cast<int>(Renderer::ShadowFilteringMode::HardwarePCF)] = "PCF 2x2 (hardware)";
        ret[static_cast<int>(Renderer::ShadowFilteringMode::PCF3x3)] = "PCF 3x3 (4 taps)";
        ret[static_cast<int>(Renderer::ShadowFilteringMode::PCFTent3x3)] = "PCF Tent 3x3 (4 taps)";
        ret[static_cast<int>(Renderer::ShadowFilteringMode::PCFTent5x5)] = "PCF Tent 5x5 (9 taps)";
        return ret;
      }()
    };

    if (int currentShadowFilteringModeIdx{static_cast<int>(gRenderer.GetShadowFilteringMode())}; ImGui::Combo("Shadow Filtering Mode", &currentShadowFilteringModeIdx, shadowFilteringModeNames.data(), static_cast<int>(std::ssize(shadowFilteringModeNames)))) {
      gRenderer.SetShadowFilteringMode(static_cast<Renderer::ShadowFilteringMode>(currentShadowFilteringModeIdx));
    }

    if (int cascadeCount{gRenderer.GetShadowCascadeCount()}; ImGui::SliderInt("Shadow Cascade Count", &cascadeCount, 1, Renderer::GetMaxShadowCascadeCount(), "%d", ImGuiSliderFlags_NoInput)) {
      gRenderer.SetShadowCascadeCount(cascadeCount);
    }

    auto const cascadeSplits{gRenderer.GetNormalizedShadowCascadeSplits()};
    auto const splitCount{std::ssize(cascadeSplits)};

    for (int i = 0; i < splitCount; i++) {
      if (float cascadeSplit{cascadeSplits[i] * 100.0f}; ImGui::SliderFloat(std::format("Split {} (percent)", i + 1).data(), &cascadeSplit, 0, 100, "%.3f", ImGuiSliderFlags_NoInput)) {
        gRenderer.SetNormalizedShadowCascadeSplit(i, cascadeSplit / 100.0f);
      }
    }

    if (bool visualizeShadowCascades{gRenderer.IsVisualizingShadowCascades()}; ImGui::Checkbox("Visualize Shadow Cascades", &visualizeShadowCascades)) {
      gRenderer.VisualizeShadowCascades(visualizeShadowCascades);
    }
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Sky")) {
    auto& activeScene{*Scene::GetActiveScene()};
    std::array constexpr static skyModeLabels{
      [] {
        std::array<char const*, 2> ret;
        ret[static_cast<int>(SkyMode::Color)] = "Color";
        ret[static_cast<int>(SkyMode::Skybox)] = "Skybox";
        return ret;
      }()
    };

    if (auto skyMode{activeScene.GetSkyMode()}; ImGui::BeginCombo("Sky Mode", skyModeLabels[static_cast<int>(skyMode)])) {
      for (auto i{0}; i < std::ssize(skyModeLabels); i++) {
        if (ImGui::Selectable(skyModeLabels[i], i == static_cast<int>(skyMode))) {
          activeScene.SetSkyMode(static_cast<SkyMode>(i));
        }
      }
      ImGui::EndCombo();
    }

    if (activeScene.GetSkyMode() == SkyMode::Color) {
      if (auto skyColor{activeScene.GetSkyColor()}; ImGui::ColorEdit3("##skyColor", skyColor.GetData())) {
        activeScene.SetSkyColor(skyColor);
      }
    } else if (activeScene.GetSkyMode() == SkyMode::Skybox) {
      if (auto skybox{activeScene.GetSkybox()}; mSkyboxPicker.Draw(skybox)) {
        activeScene.SetSkybox(skybox);
      }
    } else {
      ImGui::TextColored(ImVec4{1, 1, 0, 1}, "%s", "Unknown sky mode.");
    }

    ImGui::TreePop();
  }

  ImGui::End();
}
}
