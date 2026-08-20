#pragma once

namespace sorcery {
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
        if (ImGui::Selectable(std::format(fmt, obj.id.IsValid() ? obj.name : NULL_DISPLAY_NAME,
          mPopupId).c_str())) {
          targetObj = App::Instance().GetResourceManager().GetOrLoad<T>(obj.id);
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
    if (auto const payload{ImGui::AcceptDragDropPayload(ObjectDragDropPayload::kTypeStr.data())}) {
      if (auto const dragDropData{static_cast<ObjectDragDropPayload*>(payload->Data)};
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


template<std::derived_from<Object> T>
auto ObjectPicker<T>::QueryObjects(bool const insertNull) noexcept -> void {
  mObjects.clear();

  if constexpr (std::derived_from<T, Resource>) {
    App::Instance().GetResourceManager().GetInfoForResourcesOfType<T>(mObjects);

    std::erase_if(mObjects, [this](auto const& res_info) {
      return !Contains(res_info.name, mFilter);
    });

    std::ranges::sort(mObjects, [](auto const& lhs, auto const& rhs) {
      return lhs.name < rhs.name;
    });

    if (insertNull) {
      mObjects.insert(std::begin(mObjects),
        ResourceManager::ResourceInfo{ResourceId::Invalid(), std::string{}, rttr::type::get<T>()});
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


template<typename F>
decltype(auto) ImGuiDisabled(bool const disabled, F&& func) {
  ImGui::BeginDisabled(disabled);

  if constexpr (std::is_void_v<std::invoke_result_t<F>>) {
    std::invoke(std::forward<F>(func));
    ImGui::EndDisabled();
  } else {
    decltype(auto) result = std::invoke(std::forward<F>(func));
    ImGui::EndDisabled();
    return result;
  }
}
}
