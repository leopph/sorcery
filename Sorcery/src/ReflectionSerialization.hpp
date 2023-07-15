#pragma once

#include "YamlInclude.hpp"
#include <rttr/type.h>


namespace sorcery {
auto ReflectionSerializeToYAML(rttr::variant const& variant) -> YAML::Node;


template<typename T>
auto ReflectionSerializeToYAML(T const& obj) -> YAML::Node {
  auto const objType{ rttr::type::get<T>() };

  YAML::Node ret;

  for (auto const& prop : objType.get_properties()) {
    ret[prop.get_name().to_string()] = ReflectionSerializeToYAML(prop.get_value(obj));
  }

  return ret;
}
}
