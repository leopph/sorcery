#pragma once

#include "GUI.hpp"
#include "Reflection.hpp"

#include <cassert>


namespace sorcery::mage {
template<typename T>
auto ReflectionDisplayProperties(T& obj) -> void {
  for (auto const& prop : rttr::type::get(obj).get_properties()) {
    if (prop.get_type().is_arithmetic()) {
      auto propValue{prop.get_value(obj)};
      assert(propValue.is_valid());

      auto success{false};
      auto propValueStr{propValue.to_string(&success)};
      assert(success);

      if (ImGui::InputText(prop.get_name().data(), &propValueStr)) {
        if (rttr::variant newValue{propValueStr}; newValue.convert(prop.get_type())) {
          success = prop.set_value(obj, newValue);
          assert(success);
        }
      }
    } else if (prop.get_type().is_enumeration()) {
      auto const enumeration{prop.get_enumeration()};
      assert(enumeration.is_valid());
      auto const propValue{prop.get_value(obj)};
      assert(propValue.is_valid());

      if (ImGui::BeginCombo(prop.get_name().data(), enumeration.value_to_name(propValue).data())) {
        for (auto const& name : enumeration.get_names()) {
          if (auto const valueOfName{enumeration.name_to_value(name)}; ImGui::Selectable(name.data(), propValue == valueOfName)) {
            auto const success{prop.set_value(obj, valueOfName)};
            assert(success);
          }
        }

        ImGui::EndCombo();
      }
    }
  }
}
}
