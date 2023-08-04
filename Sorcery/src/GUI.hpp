#pragma once

#include "Core.hpp"
#include "Resources/Resource.hpp"
#include "ResourceManager.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>

#include <concepts>
#include <cstdlib>
#include <format>
#include <string>
#include <optional>


namespace sorcery {
LEOPPHAPI auto SetImGuiContext(ImGuiContext& ctx) -> void;


template<std::derived_from<Resource> T>
class ResourcePicker {
  std::vector<Guid> mGuids;
  std::vector<ObserverPtr<T>> mResources;
  std::string mFilter;
  std::string const mPopupId{std::to_string(std::rand())};
  std::string const mButtonLabel{std::format("Select##{}", mPopupId)};
  std::string const mInputTextLabel{std::format("###Filter{}", mPopupId)};

  constexpr static std::string_view NULL_RES_DISPLAY_NAME{"None"};

  auto QueryResources() noexcept -> void;

public:
  // Returns whether an assignment was made.
  [[nodiscard]] auto Draw(ObserverPtr<T>& targetRes) noexcept -> bool;
};


template<std::derived_from<Resource> T>
auto ResourcePicker<T>::QueryResources() noexcept -> void {
  mGuids.clear();
  gResourceManager.GetGuidsForResourcesOfType<T>(mGuids);

  mResources.clear();
  for (auto const& guid : mGuids) {
    mResources.emplace_back(gResourceManager.Load<T>(guid));
  }

  std::erase_if(mResources, [this](auto const res) {
    return res && !Contains(res->GetName(), mFilter);
  });

  std::ranges::sort(mResources, [](auto const lhs, auto const rhs) {
    return !lhs || (rhs && lhs->GetName() < rhs->GetName());
  });

  mResources.insert(std::begin(mResources), nullptr);
}


template<std::derived_from<Resource> T>
auto ResourcePicker<T>::Draw(ObserverPtr<T>& targetRes) noexcept -> bool {
  auto ret{false};

  if (ImGui::BeginPopup(mPopupId.c_str())) {
    if (ImGui::InputText(mInputTextLabel.c_str(), &mFilter)) {
      QueryResources();
    }

    for (auto const res : mResources) {
      if (ImGui::Selectable(std::format("{}##selectableOf{}", res ? res->GetName() : NULL_RES_DISPLAY_NAME, mPopupId).c_str())) {
        targetRes = res;
        ret = true;
      }
    }

    ImGui::EndPopup();
  }

  if (ImGui::Button(mButtonLabel.c_str())) {
    mFilter.clear();
    QueryResources();
    ImGui::OpenPopup(mPopupId.c_str());
  }

  ImGui::SameLine();
  ImGui::Text("%s", targetRes
                      ? targetRes->GetName().c_str()
                      : NULL_RES_DISPLAY_NAME.data());

  return ret;
}
}
