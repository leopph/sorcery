#include <functional>
#include <imgui.h>

#include <mono/metadata/class.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include "ObjectWrappers.hpp"
#include "Systems.hpp"


namespace leopph::editor {
auto ObjectWrapperFor<BehaviorComponent>::OnDrawProperties([[maybe_unused]] Context& context, Object& object) -> void {
  auto const& behavior{ dynamic_cast<BehaviorComponent&>(object) };

  auto const guidStr{ behavior.GetGuid().ToString() };
  if (ImGui::BeginTable(std::format("{}", guidStr).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
    auto constexpr drawComponentMemberWidget = [](std::string_view const memberName, MonoType* const memberType, std::function<void*()> const& getFunc, std::function<void(void**)> const& setFunc) {
      std::string_view const memberTypeName = mono_type_get_name(memberType);
      auto const memberClass = mono_type_get_class(memberType);

      ImGui::TableNextRow();
      if (ImGui::TableGetRowIndex() == 0) {
        ImGui::TableSetColumnIndex(0);
        ImGui::PushItemWidth(FLT_MIN);
        ImGui::TableSetColumnIndex(1);
        ImGui::PushItemWidth(-FLT_MIN);
      }

      ImGui::TableSetColumnIndex(0);
      ImGui::Text("%s", memberName.data());
      ImGui::TableSetColumnIndex(1);

      auto const widgetLabel = std::format("##WidgetForMember{}", memberName);

      if (memberClass && mono_class_is_enum(memberClass)) {
        auto const enumValues = gManagedRuntime.GetEnumValues(mono_type_get_object(gManagedRuntime.GetManagedDomain(), memberType));
        auto const numEnumValues = mono_array_length(enumValues);
        int valueAlign;
        auto const valueSize = mono_type_size(mono_class_enum_basetype(memberClass), &valueAlign);

        auto const pCurrentValueUnboxed = getFunc();
        auto const currentValueBoxed = mono_value_box(gManagedRuntime.GetManagedDomain(), memberClass, pCurrentValueUnboxed);
        auto const currentValueManagedStr = mono_object_to_string(currentValueBoxed, nullptr);
        auto const currentValueStr = mono_string_to_utf8(currentValueManagedStr);

        if (ImGui::BeginCombo(widgetLabel.c_str(), currentValueStr)) {
          for (std::size_t i{ 0 }; i < numEnumValues; i++) {
            auto pValue = mono_array_addr_with_size(enumValues, valueSize, i);
            auto const valueBoxed = mono_value_box(gManagedRuntime.GetManagedDomain(), memberClass, reinterpret_cast<void*>(pValue));

            bool selected{ true };
            for (int j{ 0 }; j < valueSize; j++) {
              if (*static_cast<char*>(pCurrentValueUnboxed) != *pValue) {
                selected = false;
                break;
              }
            }

            if (ImGui::Selectable(mono_string_to_utf8(mono_object_to_string(valueBoxed, nullptr)), selected)) {
              setFunc(reinterpret_cast<void**>(&pValue));
            }
          }

          ImGui::EndCombo();
        }
      } else if (memberTypeName == "leopph.Vector3") {
        float data[3];
        std::memcpy(data, getFunc(), sizeof(data));
        if (ImGui::DragFloat3(widgetLabel.c_str(), data, 0.1f)) {
          auto pData = &data[0];
          setFunc(reinterpret_cast<void**>(&pData));
        }
      } else if (memberTypeName == "leopph.Quaternion") {
        auto euler = static_cast<Quaternion*>(getFunc())->ToEulerAngles();
        if (ImGui::DragFloat3(widgetLabel.c_str(), euler.GetData())) {
          auto quaternion = Quaternion::FromEulerAngles(euler[0], euler[1], euler[2]);
          auto pQuaternion = &quaternion;
          setFunc(reinterpret_cast<void**>(&pQuaternion));
        }
      } else if (memberTypeName == "System.Single") {
        float data;
        std::memcpy(&data, getFunc(), sizeof(data));
        if (ImGui::DragFloat(widgetLabel.c_str(), &data)) {
          auto pData = &data;
          setFunc(reinterpret_cast<void**>(&pData));
        }
      }
    };

    auto const obj{ behavior.GetManagedObject() };
    auto const klass{ mono_object_get_class(obj) };

    void* iter{ nullptr };
    while (auto const field = mono_class_get_fields(klass, &iter)) {
      auto const refField = mono_field_get_object(gManagedRuntime.GetManagedDomain(), klass, field);

      if (gManagedRuntime.ShouldSerialize(refField)) {
        drawComponentMemberWidget(mono_field_get_name(field), mono_field_get_type(field), [field, obj] {
                                    return mono_object_unbox(mono_field_get_value_object(gManagedRuntime.GetManagedDomain(), field, obj));
                                  }, [field, obj](void** data) {
                                    mono_field_set_value(obj, field, *data);
                                  });
      }
    }

    iter = nullptr;

    while (auto const prop = mono_class_get_properties(klass, &iter)) {
      auto const refProp = mono_property_get_object(gManagedRuntime.GetManagedDomain(), klass, prop);

      if (gManagedRuntime.ShouldSerialize(refProp)) {
        drawComponentMemberWidget(mono_property_get_name(prop), mono_signature_get_return_type(mono_method_signature(mono_property_get_get_method(prop))), [prop, obj] {
                                    return mono_object_unbox(mono_property_get_value(prop, obj, nullptr, nullptr));
                                  }, [prop, obj](void** data) {
                                    mono_property_set_value(prop, reinterpret_cast<void*>(obj), data, nullptr);
                                  });
      }
    }

    ImGui::EndTable();
  }
}
}
