#pragma once

#include "Core.hpp"
#include "Math.hpp"
#include "Guid.hpp"
#include "Reflection.hpp"

// yaml-cpp incorrectly uses dllexport specifiers so we silence their warnings
#pragma warning (push)
#pragma warning (disable: 4251 4275)
#include <yaml-cpp/yaml.h>
#pragma warning (pop)


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
[[nodiscard]] auto ReflectionSerializeToYAML(T const& obj, std::function<YAML::Node(rttr::variant const&)> const& extensionFunc = {}) -> YAML::Node;
[[nodiscard]] auto ReflectionSerializeToYAML(rttr::variant const& obj, std::function<YAML::Node(rttr::variant const&)> const& extensionFunc = {}) -> YAML::Node;

template<typename T>
auto ReflectionDeserializeFromYAML(YAML::Node const& objNode, T& obj, std::function<void(YAML::Node const&, rttr::variant&)> const& extensionFunc = {}) -> void;
auto ReflectionDeserializeFromYAML(YAML::Node const& objNode, rttr::variant& obj, std::function<void(YAML::Node const&, rttr::variant&)> const& extensionFunc = {}) -> void;


namespace detail {
[[nodiscard]] auto ReflectionSerializeToYAML(rttr::variant const& variant, std::function<YAML::Node(rttr::variant const&)> const& extensionFunc) -> YAML::Node;
auto ReflectionDeserializeFromYAML(YAML::Node const& objNode, rttr::variant& variant, std::function<void(YAML::Node const&, rttr::variant&)> const& extensionFunc) -> void;
}
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
auto ReflectionSerializeToYAML(T const& obj, std::function<YAML::Node(rttr::variant const&)> const& extensionFunc) -> YAML::Node {
  YAML::Node retNode;

  for (auto const prop : rttr::type::get(obj).get_properties()) {
    retNode.push_back(detail::ReflectionSerializeToYAML(prop.get_value(obj), extensionFunc));
  }

  return retNode;
}


template<typename T>
auto ReflectionDeserializeFromYAML(YAML::Node const& objNode, T& obj, std::function<void(YAML::Node const&, rttr::variant&)> const& extensionFunc) -> void {
  for (auto const prop : rttr::type::get(obj).get_properties()) {
    rttr::variant propValue{ prop.get_value(obj) };
    detail::ReflectionDeserializeFromYAML(objNode["properties"], propValue, extensionFunc);
    prop.set_value(obj, propValue);
  }
}
}
