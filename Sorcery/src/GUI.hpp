#pragma once

#include "Core.hpp"
#include "Object.hpp"
#include "Resources/Resource.hpp"
#include "ResourceManager.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>

#include <concepts>
#include <format>
#include <string>
#include <optional>


namespace sorcery {
LEOPPHAPI auto SetImGuiContext(ImGuiContext& ctx) -> void;


template<std::derived_from<Object> T>
class ObjectPicker {
  inline static int sNextInstanceId{0};

  std::vector<Guid> mGuids;
  std::vector<ObserverPtr<T>> mObjects;
  std::string mFilter;
  int const mInstanceId{sNextInstanceId++};
  std::string const mPopupId{std::format("PopupObjectPicker{}", mInstanceId)};
  std::string const mButtonLabel{std::format("Select##ObjectPicker{}", mInstanceId)};
  std::string const mInputTextLabel{std::format("###FilterObjectPicker{}", mInstanceId)};

  constexpr static std::string_view NULL_DISPLAY_NAME{"None"};

  auto QueryObjects() noexcept -> void;

public:
  // Returns whether an assignment was made.
  [[nodiscard]] auto Draw(ObserverPtr<T>& targetObj) noexcept -> bool;
};


template<std::derived_from<Object> T>
auto ObjectPicker<T>::QueryObjects() noexcept -> void {
  if constexpr (std::derived_from<T, Resource>) {
    mGuids.clear();
    gResourceManager.GetGuidsForResourcesOfType<T>(mGuids);

    mObjects.clear();
    mObjects.reserve(mGuids.size());
    for (auto const& guid : mGuids) {
      mObjects.emplace_back(gResourceManager.Load<T>(guid));
    }
  } else {
    mObjects.clear();
    Object::FindObjectsOfType(mObjects);
  }

  std::erase_if(mObjects, [this](auto const res) {
    return res && !Contains(res->GetName(), mFilter);
  });

  std::ranges::sort(mObjects, [](auto const lhs, auto const rhs) {
    return !lhs || (rhs && lhs->GetName() < rhs->GetName());
  });

  mObjects.insert(std::begin(mObjects), nullptr);
}


template<std::derived_from<Object> T>
auto ObjectPicker<T>::Draw(ObserverPtr<T>& targetObj) noexcept -> bool {
  auto ret{false};

  if (ImGui::BeginPopup(mPopupId.c_str())) {
    if (ImGui::InputText(mInputTextLabel.c_str(), &mFilter)) {
      QueryObjects();
    }

    for (auto const obj : mObjects) {
      if (ImGui::Selectable(std::format("{}##SelectableObjectPicker{}", obj ? obj->GetName() : NULL_DISPLAY_NAME, mPopupId).c_str())) {
        targetObj = obj;
        ret = true;
      }
    }

    ImGui::EndPopup();
  }

  if (ImGui::Button(mButtonLabel.c_str())) {
    mFilter.clear();
    QueryObjects();
    ImGui::OpenPopup(mPopupId.c_str());
  }

  ImGui::SameLine();
  ImGui::Text("%s", targetObj
                      ? targetObj->GetName().c_str()
                      : NULL_DISPLAY_NAME.data());

  return ret;
}
}
