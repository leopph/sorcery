#include "GameViewWindow.hpp"

#include <imgui.h>

#include "Renderer.hpp"
#include "Util.hpp"


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

    auto const& rt{
      gRenderer.GetTemporaryRenderTarget(RenderTarget::Desc{
        .width = resolutionValues[mResIdx].width,
        .height = resolutionValues[mResIdx].height,
        .colorFormat = DXGI_FORMAT_R8G8B8A8_UNORM,
        .depthBufferBitCount = 0,
        .stencilBufferBitCount = 0,
        .sampleCount = 1,
        .debugName = "Game View RT"
      })
    };

    gRenderer.DrawAllCameras(std::addressof(rt));

    ImGui::Image(rt.GetColorSrv(), [contentRegionSize, &rt] {
      auto const scale{std::min(contentRegionSize.x / static_cast<float>(rt.GetDesc().width), contentRegionSize.y / static_cast<float>(rt.GetDesc().height))};
      return ImVec2{static_cast<float>(rt.GetDesc().width) * scale, static_cast<float>(rt.GetDesc().height) * scale};
    }());
  } else {
    ImGui::PopStyleVar();
  }
  ImGui::End();
}
}
