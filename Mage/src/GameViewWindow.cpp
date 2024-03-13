#include "GameViewWindow.hpp"

#include "engine_context.hpp"
#include "Util.hpp"

#include <imgui.h>


namespace sorcery::mage {
auto GameViewWindow::Draw(bool const gameRunning) -> void {
  ImVec2 static constexpr gameViewportMinSize{480, 270};

  ImGui::SetNextWindowSizeConstraints(gameViewportMinSize, ImVec2{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

  if (gameRunning) {
    ImGui::SetNextWindowCollapsed(false);
    ImGui::SetNextWindowFocus();
  }

  if (ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar)) {
    ImGui::PopStyleVar();

    std::array constexpr resolutionLabels{"Auto Resolution", "960x540", "1280x720", "1600x900", "1920x1080", "2560x1440", "3840x2160"};

    if (ImGui::BeginMenuBar()) {
      ImGui::SetNextItemWidth(150);

      if (ImGui::BeginCombo("##resolutionCombo", resolutionLabels[mResIdx])) {
        for (auto i{0}; i < std::ssize(resolutionLabels); i++) {
          if (ImGui::Selectable(resolutionLabels[i], mResIdx == i)) {
            mResIdx = i;
          }
        }
        ImGui::EndCombo();
      }

      ImGui::EndMenuBar();
    }

    auto const contentRegionSize = ImGui::GetContentRegionAvail();
    std::array const resolutionValues{
      Extent2D<UINT>{static_cast<UINT>(contentRegionSize.x), static_cast<UINT>(contentRegionSize.y)},
      Extent2D<UINT>{960, 540},
      Extent2D<UINT>{1280, 720},
      Extent2D<UINT>{1600, 900},
      Extent2D<UINT>{1920, 1080},
      Extent2D<UINT>{2560, 1440},
      Extent2D<UINT>{3840, 2160},
    };

    assert(std::size(resolutionLabels) == std::size(resolutionValues));

    auto const rt{
      g_engine_context.render_manager->GetTemporaryRenderTarget(rendering::RenderTarget::Desc{
        .width = resolutionValues[mResIdx].width,
        .height = resolutionValues[mResIdx].height,
        .color_format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .depth_stencil_format = std::nullopt,
        .sample_count = 1,
        .debug_name = L"Game View RT"
      })
    };

    g_engine_context.scene_renderer->SetRenderTargetOverride(rt);

    ImGui::Image(rt->GetColorTex().get(), [contentRegionSize, &rt] {
      auto const scale{std::min(contentRegionSize.x / static_cast<float>(rt->GetDesc().width), contentRegionSize.y / static_cast<float>(rt->GetDesc().height))};
      return ImVec2{static_cast<float>(rt->GetDesc().width) * scale, static_cast<float>(rt->GetDesc().height) * scale};
    }());
  } else {
    ImGui::PopStyleVar();
  }
  ImGui::End();
}
}
