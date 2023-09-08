#include "GameViewWindow.hpp"

#include <imgui.h>

#include "Core.hpp"
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

  if (ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoCollapse)) {
    ImGui::PopStyleVar();

    Extent2D<UINT> constexpr resolutions[]{{960, 540}, {1280, 720}, {1600, 900}, {1920, 1080}, {2560, 1440}, {3840, 2160}};
    constexpr char const* resolutionLabels[]{"Auto", "960x540", "1280x720", "1600x900", "1920x1080", "2560x1440", "3840x2160"};
    static int selectedRes = 0;

    ImGui::Combo("Resolution", &selectedRes, resolutionLabels, 7);

    auto const contentRegionSize = ImGui::GetContentRegionAvail();
    Extent2D const viewportRes{static_cast<UINT>(contentRegionSize.x), static_cast<UINT>(contentRegionSize.y)};

    auto const desiredRes{
      selectedRes == 0 ? viewportRes : Extent2D{resolutions[selectedRes].width, resolutions[selectedRes].height}
    };

    if (!mRenderTarget || mRenderTarget->GetDesc().width != desiredRes.width || mRenderTarget->GetDesc().height != desiredRes.height) {
      mRenderTarget = std::make_unique<RenderTarget>(RenderTarget::Desc{
        .width = static_cast<UINT>(desiredRes.width),
        .height = static_cast<UINT>(desiredRes.height),
        .colorFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        .depthBufferBitCount = 0,
        .stencilBufferBitCount = 0,
        .debugName = "Game View RenderTarget"
      });
    }

    gRenderer.DrawAllCameras(mRenderTarget.get());

    auto const frameDisplaySize{
      selectedRes == 0
        ? contentRegionSize
        : [desiredRes, contentRegionSize] {
          f32 const scale = std::min(contentRegionSize.x / static_cast<f32>(desiredRes.width), contentRegionSize.y / static_cast<f32>(desiredRes.height));
          return ImVec2(static_cast<f32>(desiredRes.width) * scale, static_cast<f32>(desiredRes.height) * scale);
        }()
    };

    ImGui::Image(mRenderTarget->GetColorSrv(), frameDisplaySize);
  } else {
    ImGui::PopStyleVar();
  }
  ImGui::End();
}
}
