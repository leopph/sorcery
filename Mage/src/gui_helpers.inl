#pragma once

#include <cassert>

#include <imgui_stdlib.h>

#include "app.hpp"
#include "object_registry.hpp"
#include "reflection.hpp"


namespace sorcery::mage {
template<std::derived_from<Object> T>
auto ObjectPicker<T>::Draw(ObjectPtr<T>& target_obj, bool const allow_null) noexcept -> bool {
  auto ret{false};

  if (ImGui::BeginPopup(popup_id_.c_str())) {
    if (ImGui::IsWindowAppearing()) {
      ImGui::SetKeyboardFocusHere();
    }

    if (ImGui::InputText(input_text_label_.c_str(), &filter_)) {
      QueryObjects(allow_null);
    }

    for (auto const obj : objects_) {
      auto constexpr fmt{"{}##SelectableObjectPicker{}"};

      if constexpr (std::derived_from<T, Resource>) {
        if (ImGui::Selectable(std::format(fmt, obj.id.IsValid() ? obj.name : kNullDisplayName,
          popup_id_).c_str())) {
          target_obj = App::Instance().GetResourceManager().Resolve<T>(obj.id);
          ret = true;
        }
      } else {
        if (ImGui::Selectable(std::format(fmt, obj ? obj->GetName() : kNullDisplayName,
          popup_id_).c_str())) {
          target_obj = obj;
          ret = true;
        }
      }
    }

    ImGui::EndPopup();
  }

  if (ImGui::Button(button_label_.c_str())) {
    filter_.clear();
    QueryObjects(allow_null);
    ImGui::OpenPopup(popup_id_.c_str());
  }

  ImGui::SameLine();
  ImGui::Text("%s", target_obj
                      ? target_obj->GetName().c_str()
                      : kNullDisplayName.data());

  if (ImGui::BeginDragDropTarget()) {
    if (auto const payload{ImGui::AcceptDragDropPayload(ObjectDragDropPayload::kTypeStr.data())}) {
      if (auto const dragDropData{static_cast<ObjectDragDropPayload*>(payload->Data)};
        dragDropData && dragDropData->ptr && rttr::type::get(*dragDropData->ptr).
        is_derived_from(rttr::type::get<T>())) {
        target_obj = MakeObjectPtr(MakeObserver(static_cast<T*>(dragDropData->ptr)));
        ret = true;
      }
    }
    ImGui::EndDragDropTarget();
  }

  return ret;
}


template<std::derived_from<Object> T>
auto ObjectPicker<T>::QueryObjects(bool const insert_null) noexcept -> void {
  objects_.clear();

  if constexpr (std::derived_from<T, Resource>) {
    App::Instance().GetResourceManager().GetInfoForResourcesOfType<T>(objects_);

    std::erase_if(objects_, [this](auto const& res_info) {
      return !Contains(res_info.name, filter_);
    });

    std::ranges::sort(objects_, [](auto const& lhs, auto const& rhs) {
      return lhs.name < rhs.name;
    });

    if (insert_null) {
      objects_.insert(std::begin(objects_),
        ResourceManager::ResourceInfo{ResourceId::Invalid(), std::string{}, rttr::type::get<T>()});
    }
  } else {
    App::Instance().GetObjectRegistry().FindObjectsOfType(objects_);

    std::erase_if(objects_, [this](auto const obj) {
      return obj && !Contains(obj->GetName(), filter_);
    });

    std::ranges::sort(objects_, [](auto const lhs, auto const rhs) {
      return !lhs || (rhs && lhs->GetName() < rhs->GetName());
    });

    if (insert_null) {
      objects_.insert(std::begin(objects_), nullptr);
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


template<typename T>
auto DrawReflectedProperties(T& obj, bool const allow_edit, bool& changed) -> void {
  for (auto const& prop : rttr::type::get(obj).get_properties()) {
    if (prop.get_type().is_arithmetic()) {
      auto propValue{prop.get_value(obj)};
      assert(propValue.is_valid());

      if (propValue.template is_type<bool>()) {
        if (auto boolValue{propValue.template get_value<bool>()}; ImGuiDisabled(!allow_edit, [&] {
          return ImGui::Checkbox(prop.get_name().data(), &boolValue);
        })) {
          [[maybe_unused]] auto const success{prop.set_value(obj, boolValue)};
          assert(success);
          changed = true;
        }
      } else {
        auto success{false};
        auto propValueStr{propValue.to_string(&success)};
        assert(success);

        if (ImGuiDisabled(!allow_edit, [&] {
          return ImGui::InputText(prop.get_name().data(), &propValueStr);
        })) {
          if (rttr::variant newValue{propValueStr}; newValue.convert(prop.get_type())) {
            success = prop.set_value(obj, newValue);
            assert(success);
            changed = true;
          }
        }
      }
    } else if (prop.get_type().is_enumeration()) {
      auto const enumeration{prop.get_enumeration()};
      assert(enumeration.is_valid());
      auto const propValue{prop.get_value(obj)};
      assert(propValue.is_valid());

      if (ImGuiDisabled(!allow_edit, [&] {
        return ImGui::BeginCombo(prop.get_name().data(), enumeration.value_to_name(propValue).data());
      })) {
        for (auto const& name : enumeration.get_names()) {
          if (auto const valueOfName{enumeration.name_to_value(name)}; ImGui::Selectable(name.data(),
            propValue == valueOfName)) {
            [[maybe_unused]] auto const success{prop.set_value(obj, valueOfName)};
            assert(success);
            changed = true;
          }
        }

        ImGui::EndCombo();
      }
    }
  }
}
}
