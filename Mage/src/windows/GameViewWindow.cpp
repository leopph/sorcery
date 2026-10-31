#include "GameViewWindow.hpp"

#include <algorithm>

#include "../gui_helpers.hpp"
#include "../imgui_texture_references.hpp"
#include "rendering/scene_renderer.hpp"
#include "sorcery/util.hpp"
#include "sorcery/world.hpp"
#include "sorcery/host/app.hpp"
#include "sorcery/host/game_views.hpp"
#include "sorcery/scene/CameraComponent.hpp"


namespace sorcery::mage {
GameViewWindow::GameViewWindow(ImGuiTextureReferences& tex_refs) :
  tex_refs_{&tex_refs} {}


auto GameViewWindow::Draw(World const& world, bool const game_is_running) -> void {
  ImVec2 static constexpr game_viewport_min_size{480, 270};

  ImGui::SetNextWindowSizeConstraints(game_viewport_min_size, ImVec2{
    std::numeric_limits<float>::max(), std::numeric_limits<float>::max()
  });

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

  if (game_is_running) {
    ImGui::SetNextWindowCollapsed(false);
    ImGui::SetNextWindowFocus();
  }

  auto const is_visible{ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar)};

  was_visible_ = is_visible;

  if (!is_visible) {
    ImGui::PopStyleVar();
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

  if (!rt_ || resolution_values[resolution_mode_idx_].width != rt_->GetDesc().width ||
      resolution_values[resolution_mode_idx_].height != rt_->GetDesc().height) {
    rt_ = rendering::RenderTarget::New(App::Instance().GetGraphicsDevice(), rendering::RenderTarget::Desc{
      resolution_values[resolution_mode_idx_].width, resolution_values[resolution_mode_idx_].height,
      DXGI_FORMAT_R8G8B8A8_UNORM, std::nullopt, 1, L"Game View RT"
    });
  }

  // Nothing clears the target when no view renders into it, so it would show a stale frame.
  if (std::ranges::none_of(world.GetCameras(), [](auto const cam) { return !cam->GetRenderTarget(); })) {
    constexpr auto no_camera_text{"No cameras rendering"};
    auto const text_size{ImGui::CalcTextSize(no_camera_text)};
    ImGui::SetCursorPos(ImVec2{
      ImGui::GetCursorPosX() + (content_region_size.x - text_size.x) * 0.5f,
      ImGui::GetCursorPosY() + (content_region_size.y - text_size.y) * 0.5f
    });
    ImGui::TextDisabled("%s", no_camera_text);
  } else {
    ImGui::Image(tex_refs_->Reference(rt_), [content_region_size, this] {
      auto const& desc{rt_->GetDesc()};
      auto const scale{
        std::min(content_region_size.x / static_cast<float>(desc.width),
          content_region_size.y / static_cast<float>(desc.height))
      };
      return ImVec2{static_cast<float>(desc.width) * scale, static_cast<float>(desc.height) * scale};
    }());
  }

  ImGui::End();
}


auto GameViewWindow::AddViews(World const& world, rendering::ViewList& views) const -> void {
  // Cameras with their own target always render, since their output can be shown outside the game view.
  CollectGameViews(world, was_visible_ ? rt_ : nullptr, views);
}
}
