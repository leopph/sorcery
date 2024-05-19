#pragma once

#include "Core.hpp"
#include "engine_context.hpp"
#include "Object.hpp"
#include "ResourceManager.hpp"
#include "Resources/Resource.hpp"
#include "Util.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>

#include <concepts>
#include <format>
#include <optional>
#include <string>
#include <type_traits>


namespace sorcery {
LEOPPHAPI auto SetImGuiContext(ImGuiContext& ctx) -> void;


namespace detail {
class ObjectPickerBase {
  static int sNextInstanceId;

protected:
  [[nodiscard]] LEOPPHAPI static auto GetNextInstanceId() noexcept -> int;
};
}


template<std::derived_from<Object> T>
class ObjectPicker : detail::ObjectPickerBase {
  using StoredType = std::conditional_t<std::derived_from<T, Resource>, ResourceManager::ResourceInfo, T*>;

  std::vector<StoredType> mObjects;
  std::string mFilter;
  int const mInstanceId{GetNextInstanceId()};
  std::string const mPopupId{std::format("PopupObjectPicker{}", mInstanceId)};
  std::string const mButtonLabel{std::format("Select##ObjectPicker{}", mInstanceId)};
  std::string const mInputTextLabel{std::format("###FilterObjectPicker{}", mInstanceId)};

  constexpr static std::string_view NULL_DISPLAY_NAME{"None"};

  auto QueryObjects(bool insertNull) noexcept -> void;

public:
  // Returns whether an assignment was made.
  [[nodiscard]] auto Draw(T*& targetObj, bool allowNull = true) noexcept -> bool;
};


struct ObjectDragDropData {
  LEOPPHAPI static std::string_view const TYPE_STR;
  Object* ptr;
};


template<std::derived_from<Object> T>
auto ObjectPicker<T>::QueryObjects(bool const insertNull) noexcept -> void {
  mObjects.clear();

  if constexpr (std::derived_from<T, Resource>) {
    g_engine_context.resource_manager->GetInfoForResourcesOfType<T>(mObjects);

    std::erase_if(mObjects, [this](auto const& res_info) {
      return !Contains(res_info.name, mFilter);
    });

    std::ranges::sort(mObjects, [](auto const& lhs, auto const& rhs) {
      return lhs.name < rhs.name;
    });

    if (insertNull) {
      mObjects.insert(std::begin(mObjects),
        ResourceManager::ResourceInfo{Guid::Invalid(), std::string{}, rttr::type::get<T>()});
    }
  } else {
    Object::FindObjectsOfType(mObjects);

    std::erase_if(mObjects, [this](auto const obj) {
      return obj && !Contains(obj->GetName(), mFilter);
    });

    std::ranges::sort(mObjects, [](auto const lhs, auto const rhs) {
      return !lhs || (rhs && lhs->GetName() < rhs->GetName());
    });

    if (insertNull) {
      mObjects.insert(std::begin(mObjects), nullptr);
    }
  }
}


template<std::derived_from<Object> T>
auto ObjectPicker<T>::Draw(T*& targetObj, bool const allowNull) noexcept -> bool {
  auto ret{false};

  if (ImGui::BeginPopup(mPopupId.c_str())) {
    if (ImGui::IsWindowAppearing()) {
      ImGui::SetKeyboardFocusHere();
    }

    if (ImGui::InputText(mInputTextLabel.c_str(), &mFilter)) {
      QueryObjects(allowNull);
    }

    for (auto const obj : mObjects) {
      auto constexpr fmt{"{}##SelectableObjectPicker{}"};

      if constexpr (std::derived_from<T, Resource>) {
        if (ImGui::Selectable(std::format(fmt, obj.guid.IsValid() ? obj.name : NULL_DISPLAY_NAME,
          mPopupId).c_str())) {
          targetObj = g_engine_context.resource_manager->GetOrLoad<T>(obj.guid);
          ret = true;
        }
      } else {
        if (ImGui::Selectable(std::format(fmt, obj ? obj->GetName() : NULL_DISPLAY_NAME,
          mPopupId).c_str())) {
          targetObj = obj;
          ret = true;
        }
      }
    }

    ImGui::EndPopup();
  }

  if (ImGui::Button(mButtonLabel.c_str())) {
    mFilter.clear();
    QueryObjects(allowNull);
    ImGui::OpenPopup(mPopupId.c_str());
  }

  ImGui::SameLine();
  ImGui::Text("%s", targetObj
                      ? targetObj->GetName().c_str()
                      : NULL_DISPLAY_NAME.data());

  if (ImGui::BeginDragDropTarget()) {
    if (auto const payload{ImGui::AcceptDragDropPayload(ObjectDragDropData::TYPE_STR.data())}) {
      if (auto const dragDropData{static_cast<ObjectDragDropData*>(payload->Data)};
        dragDropData && dragDropData->ptr && rttr::type::get(*dragDropData->ptr).
        is_derived_from(rttr::type::get<T>())) {
        targetObj = static_cast<T*>(dragDropData->ptr);
        ret = true;
      }
    }
    ImGui::EndDragDropTarget();
  }

  return ret;
}
}
