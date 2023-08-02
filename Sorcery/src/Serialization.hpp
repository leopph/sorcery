#pragma once

#include "Core.hpp"
#include "Math.hpp"
#include "Guid.hpp"
#include "Reflection.hpp"
#include "Object.hpp"

// yaml-cpp incorrectly uses dllexport specifiers so we silence their warnings
#pragma warning (push)
#pragma warning (disable: 4251 4275)
#include <yaml-cpp/yaml.h>
#pragma warning (pop)

#include <cassert>
#include <type_traits>


namespace YAML {
template<typename T, std::size_t N>
struct convert<sorcery::Vector<T, N>> {
  static auto encode(sorcery::Vector<T, N> const& v) -> Node;
  static auto decode(Node const& node, sorcery::Vector<T, N>& v) -> bool;
};


template<>
struct convert<sorcery::Quaternion> {
  LEOPPHAPI static auto encode(sorcery::Quaternion const& q) -> Node;
  LEOPPHAPI static auto decode(Node const& node, sorcery::Quaternion& q) -> bool;
};


template<>
struct convert<sorcery::Guid> {
  LEOPPHAPI static auto encode(sorcery::Guid const& guid) -> Node;
  LEOPPHAPI static auto decode(Node const& node, sorcery::Guid& guid) -> bool;
};
}


namespace sorcery {
template<typename T>
[[nodiscard]] auto ReflectionSerializeToYaml(T const& obj, std::function<YAML::Node(rttr::variant const&)> const& extensionFunc = {}) -> YAML::Node;
[[nodiscard]] LEOPPHAPI auto ReflectionSerializeToYaml(rttr::variant const& v, std::function<YAML::Node(rttr::variant const&)> const& extensionFunc = {}) -> YAML::Node;

template<typename T> requires (!std::derived_from<T, Object>)
auto ReflectionDeserializeFromYaml(YAML::Node const& node, T& obj, std::function<void(YAML::Node const&, rttr::variant&)> const& extensionFunc = {}) -> void;
LEOPPHAPI auto ReflectionDeserializeFromYaml(YAML::Node const& node, Object& obj, std::function<void(YAML::Node const&, rttr::variant&)> const& extensionFunc = {}) -> void;
LEOPPHAPI auto ReflectionDeserializeFromYaml(YAML::Node const& node, rttr::variant& v, std::function<void(YAML::Node const&, rttr::variant&)> const& extensionFunc = {}) -> void;
}


namespace YAML {
template<typename T, std::size_t N>
auto convert<sorcery::Vector<T, N>>::encode(sorcery::Vector<T, N> const& v) -> Node {
  Node node;
  node.SetStyle(EmitterStyle::Flow);
  for (std::size_t i = 0; i < N; i++) {
    node.push_back(v[i]);
  }
  return node;
}


template<typename T, std::size_t N>
auto convert<sorcery::Vector<T, N>>::decode(Node const& node, sorcery::Vector<T, N>& v) -> bool {
  if (!node.IsSequence() || node.size() != N) {
    return false;
  }
  for (std::size_t i = 0; i < N; i++) {
    v[i] = node[i].as<T>();
  }
  return true;
}
}


namespace sorcery {
template<typename T>
auto ReflectionSerializeToYaml(T const& obj, std::function<YAML::Node(rttr::variant const&)> const& extensionFunc) -> YAML::Node {
  if constexpr (std::is_base_of_v<Object, T>) {
    YAML::Node ret;
    for (auto const& prop : rttr::type::get(obj).get_properties()) {
      auto const propValue{ prop.get_value(obj) };
      assert(propValue.is_valid());
      ret[prop.get_name().to_string()] = ReflectionSerializeToYaml(propValue, extensionFunc);
    }
    return ret;
  } else {
    return ReflectionSerializeToYaml(rttr::variant{ std::ref(obj) }, extensionFunc);
  }
}


template<typename T> requires (!std::derived_from<T, Object>)
auto ReflectionDeserializeFromYaml(YAML::Node const& node, T& obj, std::function<void(YAML::Node const&, rttr::variant&)> const& extensionFunc) -> void {
  rttr::variant v{ std::ref(obj) };
  ReflectionDeserializeFromYaml(node, v, extensionFunc);
}
}
