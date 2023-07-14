#include "GameViewWindow.hpp"

#include <imgui.h>

#include "Core.hpp"
#include "Renderer.hpp"
#include "Systems.hpp"
#include "Util.hpp"


namespace sorcery::mage {
auto GameViewWindow::Draw(bool const gameRunning, EditorCamera const& editorCam) -> void {
  ImVec2 static constexpr gameViewportMinSize{ 480, 270 };

  ImGui::SetNextWindowSize(gameViewportMinSize, ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSizeConstraints(gameViewportMinSize, ImGui::GetMainViewport()->WorkSize);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

  if (gameRunning) {
    ImGui::SetNextWindowCollapsed(false);
    ImGui::SetNextWindowFocus();
  }

  if (ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoCollapse)) {
    ImGui::PopStyleVar();

    Extent2D<u32> constexpr resolutions[]{ { 960, 540 }, { 1280, 720 }, { 1600, 900 }, { 1920, 1080 }, { 2560, 1440 }, { 3840, 2160 } };
    constexpr char const* resolutionLabels[]{ "Auto", "960x540", "1280x720", "1600x900", "1920x1080", "2560x1440", "3840x2160" };
    static int selectedRes = 0;

    ImGui::Combo("Resolution", &selectedRes, resolutionLabels, 7);

    auto const contentRegionSize = ImGui::GetContentRegionAvail();
    Extent2D const viewportRes{ static_cast<u32>(contentRegionSize.x), static_cast<u32>(contentRegionSize.y) };

    auto const desiredRes{ selectedRes == 0 ? viewportRes : Extent2D<u32>{ resolutions[selectedRes].width, resolutions[selectedRes].height } };

    if (!mHdrRenderTarget || mHdrRenderTarget->GetDesc().width != desiredRes.width || mHdrRenderTarget->GetDesc().height != desiredRes.height) {
      mHdrRenderTarget = std::make_unique<RenderTarget>(RenderTarget::Desc{
        .width = desiredRes.width,
        .height = desiredRes.height,
        .colorFormat = DXGI_FORMAT_R16G16B16A16_FLOAT,
        .depthStencilFormat = DXGI_FORMAT_D32_FLOAT,
        .debugName = "Game View HDR"
      });
    }

    if (!mFinalRenderTarget || mFinalRenderTarget->GetDesc().width != desiredRes.width || mFinalRenderTarget->GetDesc().height != desiredRes.height) {
      mFinalRenderTarget = std::make_unique<RenderTarget>(RenderTarget::Desc{
        .width = desiredRes.width,
        .height = desiredRes.height,
        .colorFormat = DXGI_FORMAT_R8G8B8A8_UNORM,
        .depthStencilFormat = std::nullopt,
        .debugName = "Game View Final"
      });
    }

    auto const frameDisplaySize{
      selectedRes == 0
        ? contentRegionSize
        : [desiredRes, contentRegionSize] {
          f32 const scale = std::min(contentRegionSize.x / static_cast<f32>(desiredRes.width), contentRegionSize.y / static_cast<f32>(desiredRes.height));
          return ImVec2(static_cast<f32>(desiredRes.width) * scale, static_cast<f32>(desiredRes.height) * scale);
        }()
    };

    gRenderer.DrawCamera(editorCam, mHdrRenderTarget.get());
    // Do post process TODO
    ImGui::Image(mFinalRenderTarget->GetColorSrv(), frameDisplaySize);
  } else {
    ImGui::PopStyleVar();
  }
  ImGui::End();
}
}
