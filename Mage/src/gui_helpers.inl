#pragma once

#include <cassert>
#include <utility>

#include <imgui_stdlib.h>

#include "sorcery/reflection.hpp"
#include "sorcery/host/app.hpp"
#include "sorcery/host/object_registry.hpp"


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

      if constexpr (std::derived_from<T, Asset>) {
        if (ImGui::Selectable(std::format(fmt, obj.id.IsValid() ? obj.name : kNullDisplayName,
          popup_id_).c_str())) {
          target_obj = App::Instance().GetAssetManager().Resolve<T>(obj.id);
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

  auto const label = target_obj ? target_obj->GetName() : kNullDisplayName;
  ImGui::TextUnformatted(label.data(), label.data() + label.size());

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

  if constexpr (std::derived_from<T, Asset>) {
    App::Instance().GetAssetManager().GetInfoForAssetsOfType<T>(objects_);

    std::erase_if(objects_, [this](auto const& asset_info) {
      return !Contains(asset_info.name, filter_);
    });

    std::ranges::sort(objects_, [](auto const& lhs, auto const& rhs) {
      return lhs.name < rhs.name;
    });

    if (insert_null) {
      objects_.insert(std::begin(objects_),
        AssetManager::AssetInfo{AssetId::Invalid(), std::string{}, rttr::type::get<T>()});
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
  DrawReflectedProperties(obj, allow_edit, changed, []([[maybe_unused]] rttr::property const& prop) {
    return true;
  });
}


template<typename T, typename Predicate>
auto DrawReflectedProperties(T& obj, bool const allow_edit, bool& changed, Predicate&& pred) -> void {
  DrawReflectedProperties(rttr::type::get(obj), obj, allow_edit, changed, std::forward<Predicate>(pred));
}


template<typename T, typename Predicate>
auto DrawReflectedProperties(rttr::type const& type, T& obj, bool const allow_edit, bool& changed,
                             Predicate&& pred) -> void {
  auto const in_table = ImGui::TableGetColumnCount() >= 2;

  for (auto const& prop : type.get_properties()) {
    if (!pred(prop)) {
      continue;
    }

    if (auto const hidden = prop.get_metadata(reflection::kHideInInspector);
      hidden.is_valid() && hidden.is_type<bool>() && hidden.get_value<bool>()) {
      continue;
    }

    if (!prop.get_type().is_arithmetic() &&
        !prop.get_type().is_enumeration()) {
      continue;
    }

    auto const prop_name = prop.get_name().to_string();

    if (in_table) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted(prop_name.c_str());
      ImGui::TableSetColumnIndex(1);
    }

    ImGui::PushID(prop_name.c_str());
    auto const* const label = in_table ? "##value" : prop_name.c_str();

    // If arithmetic

    if (prop.get_type().is_arithmetic()) {
      auto prop_value = prop.get_value(obj);
      assert(prop_value.is_valid());

      // If bool

      if (prop_value.template is_type<bool>()) {
        if (auto bool_val = prop_value.template get_value<bool>(); ImGuiDisabled(!allow_edit, [&] {
          return ImGui::Checkbox(label, &bool_val);
        })) {
          [[maybe_unused]] auto const success = prop.set_value(obj, bool_val);
          assert(success);
          changed = true;
        }
      } else {
        // This applies to all controls except the bool checkbox
        ImGui::SetNextItemWidth(-FLT_MIN);

        // If float

        if (prop_value.template is_type<float>()) {
          if (auto float_val = prop_value.template get_value<float>(); ImGuiDisabled(!allow_edit, [&] {
            return ImGui::DragFloat(label, &float_val);
          })) {
            [[maybe_unused]] auto const success = prop.set_value(obj, float_val);
            assert(success);
            changed = true;
          }
        } else {
          // If other arithmetic

          auto success = false;
          auto prop_value_str = prop_value.to_string(&success);
          assert(success);

          if (ImGuiDisabled(!allow_edit, [&] {
            return ImGui::InputText(label, &prop_value_str);
          })) {
            if (rttr::variant new_val{prop_value_str}; new_val.convert(prop.get_type())) {
              success = prop.set_value(obj, new_val);
              assert(success);
              changed = true;
            }
          }
        }
      }
    } else {
      // If not arithmetic

      ImGui::SetNextItemWidth(-FLT_MIN);

      // If enum

      if (prop.get_type().is_enumeration()) {
        auto const enumeration = prop.get_enumeration();
        assert(enumeration.is_valid());

        auto const prop_val = prop.get_value(obj);
        assert(prop_val.is_valid());

        if (ImGuiDisabled(!allow_edit, [&] {
          return ImGui::BeginCombo(label, enumeration.value_to_name(prop_val).data());
        })) {
          for (auto const& name : enumeration.get_names()) {
            if (auto const value_of_name = enumeration.name_to_value(name); ImGui::Selectable(name.data(),
              prop_val == value_of_name)) {
              [[maybe_unused]] auto const success = prop.set_value(obj, value_of_name);
              assert(success);
              changed = true;
            }
          }

          ImGui::EndCombo();
        }
      }
    }

    ImGui::PopID();
  }
}
}
