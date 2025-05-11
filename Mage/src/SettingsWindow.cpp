#include "SettingsWindow.hpp"

#include "app.hpp"
#include "EditorApp.hpp"
#include "editor_gui.hpp"
#include "MemoryAllocation.hpp"
#include "scene_renderer.hpp"
#include "Timing.hpp"

#include <limits>


namespace sorcery::mage {
SettingsWindow::SettingsWindow(EditorApp& app, StandaloneCamera& sceneViewCam) :
  mApp{std::addressof(app)},
  mSceneViewCam{std::addressof(sceneViewCam)} {}


auto SettingsWindow::Draw() -> void {
  ImGui::SetNextWindowSizeConstraints(ImVec2{200, 200}, ImVec2{
    std::numeric_limits<float>::max(), std::numeric_limits<float>::max()
  });

  if (std::string windowName; !ImGui::Begin(windowName.append(TITLE).append("##Window").c_str())) {
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
    if (int targetFrameRate{timing::GetTargetFrameRate()}; ImGui::DragInt("Target Frame Rate", &targetFrameRate, 1, 30,
      std::numeric_limits<int>::max(), "%d", ImGuiSliderFlags_AlwaysClamp)) {
      timing::SetTargetFrameRate(targetFrameRate);
    }
    ImGui::EndDisabled();

    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Rendering")) {
    if (auto preciseColor{App::Instance().GetSceneRenderer().IsUsingPreciseColorFormat()}; ImGui::Checkbox(
      "Use Precise Color Buffer Format", &preciseColor)) {
      App::Instance().GetSceneRenderer().SetUsePreciseColorFormat(preciseColor);
    }

    if (auto gamma{App::Instance().GetSceneRenderer().GetGamma()}; ImGui::DragFloat("Gamma", &gamma, 0.01f, 0.01f, 5.0f,
      "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
      App::Instance().GetSceneRenderer().SetGamma(gamma);
    }

    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Lighting")) {
    if (auto color{Scene::GetActiveScene()->GetAmbientLightVector()}; ImGui::ColorEdit3("Ambient Light",
      color.GetData())) {
      Scene::GetActiveScene()->SetAmbientLightVector(color);
    }

    if (auto ssaoEnabled{App::Instance().GetSceneRenderer().IsSsaoEnabled()}; ImGui::Checkbox(
      "Screen Space Ambient Occlusion", &ssaoEnabled)) {
      App::Instance().GetSceneRenderer().SetSsaoEnabled(ssaoEnabled);
    }

    ImGui::BeginDisabled(!App::Instance().GetSceneRenderer().IsSsaoEnabled());
    ImGui::Indent();

    auto ssaoParams{App::Instance().GetSceneRenderer().GetSsaoParams()};

    if (ImGui::DragFloat("Radius", &ssaoParams.radius, 0.01f, std::numeric_limits<float>::min(),
      std::numeric_limits<float>::max() / 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
      App::Instance().GetSceneRenderer().SetSsaoParams(ssaoParams);
    }
    if (ImGui::DragFloat("Bias", &ssaoParams.bias, 0.001f, 0, std::numeric_limits<float>::max() / 2.0f, "%.3f",
      ImGuiSliderFlags_AlwaysClamp)) {
      App::Instance().GetSceneRenderer().SetSsaoParams(ssaoParams);
    }
    if (ImGui::DragFloat("Power", &ssaoParams.power, 0.01f, std::numeric_limits<float>::min(),
      std::numeric_limits<float>::max() / 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
      App::Instance().GetSceneRenderer().SetSsaoParams(ssaoParams);
    }
    if (ImGui::DragInt("Sample Count", &ssaoParams.sample_count, 1, 0, std::numeric_limits<int>::max(), "%d",
      ImGuiSliderFlags_AlwaysClamp)) {
      App::Instance().GetSceneRenderer().SetSsaoParams(ssaoParams);
    }

    ImGui::Unindent();
    ImGui::EndDisabled();

    if (auto ssr_enabled{App::Instance().GetSceneRenderer().IsSsrEnabled()};
      ImGui::Checkbox("Screen Space Reflections", &ssr_enabled)) {
      App::Instance().GetSceneRenderer().SetSsrEnabled(ssr_enabled);
    }

    ImGui::BeginDisabled(!App::Instance().GetSceneRenderer().IsSsrEnabled());
    ImGui::Indent();

    auto ssr_params{App::Instance().GetSceneRenderer().GetSsrParams()};

    if (ImGui::DragFloat("Roughness Threshold", &ssr_params.max_roughness, 0.001f, 0, 1, "%.3f",
      ImGuiSliderFlags_AlwaysClamp)) {
      App::Instance().GetSceneRenderer().SetSsrParams(ssr_params);
    }

    if (ImGui::DragFloat("Thickness", &ssr_params.thickness_vs, 1.0f, 0,
      std::numeric_limits<float>::max() / 2.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp)) {
      App::Instance().GetSceneRenderer().SetSsrParams(ssr_params);
    }

    if (ImGui::DragFloat("Stride", &ssr_params.stride, 1.0f, 1.0f,
      std::numeric_limits<float>::max() / 2.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp)) {
      App::Instance().GetSceneRenderer().SetSsrParams(ssr_params);
    }

    if (ImGui::DragFloat("Jitter", &ssr_params.jitter, 0.001f, 0.f,
      std::numeric_limits<float>::max() / 2.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
      App::Instance().GetSceneRenderer().SetSsrParams(ssr_params);
    }

    if (ImGui::DragFloat("Max Trace Distance", &ssr_params.max_trace_dist_vs, 1.0f, 1.0f,
      std::numeric_limits<float>::max() / 2.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp)) {
      App::Instance().GetSceneRenderer().SetSsrParams(ssr_params);
    }

    if (ImGui::DragFloat("Ray Start Bias", &ssr_params.ray_start_bias_vs, 0.01f, 0.0f,
      std::numeric_limits<float>::max() / 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
      App::Instance().GetSceneRenderer().SetSsrParams(ssr_params);
    }

    ImGui::Unindent();
    ImGui::EndDisabled();
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Shadows")) {
    float shadowDistance{App::Instance().GetSceneRenderer().GetShadowDistance()};
    if (ImGui::DragFloat("Shadow Distance", &shadowDistance, 1, 0, std::numeric_limits<float>::max(), "%.0f",
      ImGuiSliderFlags_AlwaysClamp)) {
      App::Instance().GetSceneRenderer().SetShadowDistance(shadowDistance);
    }

    auto constexpr shadowFilteringModeNames{
      [] {
        std::array<char const*, 5> ret{};
        ret[static_cast<int>(rendering::ShadowFilteringMode::kNone)] = "No Filtering";
        ret[static_cast<int>(rendering::ShadowFilteringMode::kHardwarePcf)] = "PCF 2x2 (hardware)";
        ret[static_cast<int>(rendering::ShadowFilteringMode::kPcf3X3)] = "PCF 3x3 (4 taps)";
        ret[static_cast<int>(rendering::ShadowFilteringMode::kPcfTent3X3)] = "PCF Tent 3x3 (4 taps)";
        ret[static_cast<int>(rendering::ShadowFilteringMode::kPcfTent5X5)] = "PCF Tent 5x5 (9 taps)";
        return ret;
      }()
    };

    if (auto currentShadowFilteringModeIdx{
        static_cast<int>(App::Instance().GetSceneRenderer().GetShadowFilteringMode())
      };
      ImGui::Combo("Shadow Filtering Mode", &currentShadowFilteringModeIdx, shadowFilteringModeNames.data(),
        static_cast<int>(std::ssize(shadowFilteringModeNames)))) {
      App::Instance().GetSceneRenderer().SetShadowFilteringMode(
        static_cast<rendering::ShadowFilteringMode>(currentShadowFilteringModeIdx));
    }

    if (auto cascadeCount{static_cast<int>(App::Instance().GetSceneRenderer().GetShadowCascadeCount())};
      ImGui::SliderInt("Shadow Cascade Count", &cascadeCount, 1, rendering::SceneRenderer::GetMaxShadowCascadeCount(),
        "%d", ImGuiSliderFlags_NoInput)) {
      App::Instance().GetSceneRenderer().SetShadowCascadeCount(cascadeCount);
    }

    auto const cascadeSplits{App::Instance().GetSceneRenderer().GetNormalizedShadowCascadeSplits()};
    auto const splitCount{std::ssize(cascadeSplits)};

    for (auto i = 0; i < splitCount; i++) {
      if (float cascadeSplit{cascadeSplits[i] * 100.0f}; ImGui::SliderFloat(
        std::format("Split {} (percent)", i + 1).data(), &cascadeSplit, 0, 100, "%.3f", ImGuiSliderFlags_NoInput)) {
        App::Instance().GetSceneRenderer().SetNormalizedShadowCascadeSplit(i, cascadeSplit / 100.0f);
      }
    }

    if (bool visualizeShadowCascades{App::Instance().GetSceneRenderer().IsVisualizingShadowCascades()}; ImGui::Checkbox(
      "Visualize Shadow Cascades", &visualizeShadowCascades)) {
      App::Instance().GetSceneRenderer().VisualizeShadowCascades(visualizeShadowCascades);
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

    if (auto skyMode{activeScene.GetSkyMode()};
      ImGui::BeginCombo("Sky Mode", skyModeLabels[static_cast<int>(skyMode)])) {
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
