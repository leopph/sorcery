#include "GameViewWindow.hpp"

#include "app.hpp"
#include "editor_gui.hpp"
#include "scene_renderer.hpp"
#include "Util.hpp"


namespace sorcery::mage {
auto GameViewWindow::Draw(bool const game_is_running) -> void {
  ImVec2 static constexpr game_viewport_min_size{480, 270};

  ImGui::SetNextWindowSizeConstraints(game_viewport_min_size, ImVec2{
    std::numeric_limits<float>::max(), std::numeric_limits<float>::max()
  });

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

  if (game_is_running) {
    ImGui::SetNextWindowCollapsed(false);
    ImGui::SetNextWindowFocus();
  }

  if (!ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar)) {
    ImGui::PopStyleVar();

    if (App::Instance().GetSceneRenderer().GetRenderTargetOverride() == rt_override_) {
      App::Instance().GetSceneRenderer().SetRenderTargetOverride(nullptr);
    }

    ImGui::End();
    return;
  }

  ImGui::PopStyleVar();

  std::array constexpr resolution_labels{
    "Auto Resolution", "960x540", "1280x720", "1600x900", "1920x1080", "2560x1440", "3840x2160"
  };

  if (ImGui::BeginMenuBar()) {
    ImGui::SetNextItemWidth(150);

    if (ImGui::BeginCombo("##resolutionCombo", resolution_labels[resolution_mode_idx_])) {
      for (auto i{0}; i < std::ssize(resolution_labels); i++) {
        if (ImGui::Selectable(resolution_labels[i], resolution_mode_idx_ == i)) {
          resolution_mode_idx_ = i;
        }
      }
      ImGui::EndCombo();
    }

    ImGui::EndMenuBar();
  }

  auto const content_region_size = ImGui::GetContentRegionAvail();

  std::array const resolution_values{
    Extent2D<UINT>{static_cast<UINT>(content_region_size.x), static_cast<UINT>(content_region_size.y)},
    Extent2D<UINT>{960, 540}, Extent2D<UINT>{1280, 720}, Extent2D<UINT>{1600, 900}, Extent2D<UINT>{1920, 1080},
    Extent2D<UINT>{2560, 1440}, Extent2D<UINT>{3840, 2160},
  };

  static_assert(std::size(resolution_labels) == std::size(resolution_values));

  if (!rt_override_ || resolution_values[resolution_mode_idx_].width != rt_override_->GetDesc().width ||
      resolution_values[resolution_mode_idx_].height != rt_override_->GetDesc().height) {
    rt_override_ = rendering::RenderTarget::New(App::Instance().GetGraphicsDevice(), rendering::RenderTarget::Desc{
      resolution_values[resolution_mode_idx_].width, resolution_values[resolution_mode_idx_].height,
      DXGI_FORMAT_R8G8B8A8_UNORM, std::nullopt, 1, L"Game View RT"
    });
  }

  if (App::Instance().GetSceneRenderer().GetRenderTargetOverride() != rt_override_) {
    App::Instance().GetSceneRenderer().SetRenderTargetOverride(rt_override_);
  }

  ImGui::Image(std::bit_cast<ImTextureID>(rt_override_->GetColorTex().get()), [content_region_size, this] {
    auto const& desc{rt_override_->GetDesc()};
    auto const scale{
      std::min(content_region_size.x / static_cast<float>(desc.width),
        content_region_size.y / static_cast<float>(desc.height))
    };
    return ImVec2{static_cast<float>(desc.width) * scale, static_cast<float>(desc.height) * scale};
  }());

  ImGui::End();
}
}
