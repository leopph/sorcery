#pragma once

#include <algorithm>
#include <bit>
#include <concepts>
#include <limits>
#include <span>
#include <string_view>
#include <type_traits>
#include <vector>

// yaml-cpp incorrectly uses dllexport specifiers so we silence their warnings
#pragma warning (push)
#pragma warning (disable: 4251 4275)
#include <yaml-cpp/yaml.h>
#pragma warning (pop)

#include "Core.hpp"
#include "Guid.hpp"
#include "Math.hpp"
#include "Object.hpp"
#include "Reflection.hpp"
#include "resource_reference.hpp"


namespace YAML {
template<typename T, std::size_t N>
struct convert<sorcery::Vector<T, N>> {
  static auto encode(sorcery::Vector<T, N> const& v) -> Node;
  static auto decode(Node const& node, sorcery::Vector<T, N>& v) -> bool;
};


template<>
struct convert<sorcery::Quaternion> {
  SORCERYAPI static auto encode(sorcery::Quaternion const& q) -> Node;
  SORCERYAPI static auto decode(Node const& node, sorcery::Quaternion& q) -> bool;
};


template<>
struct convert<sorcery::Guid> {
  SORCERYAPI static auto encode(sorcery::Guid const& guid) -> Node;
  SORCERYAPI static auto decode(Node const& node, sorcery::Guid& guid) -> bool;
};
}


namespace sorcery {
// Reflection-based serialization to YAML

template<typename T> requires (!std::derived_from<T, Object>)
[[nodiscard]]
auto ReflectionSerializeToYaml(
  T const& obj,
  YamlSerializeContext const& ctx,
  std::function<YAML::Node(rttr::variant const&, YamlSerializeContext const&)> const& extension_func = {}
) noexcept -> YAML::Node;

[[nodiscard]] SORCERYAPI
auto ReflectionSerializeToYaml(
  Object const& obj,
  YamlSerializeContext const& ctx,
  std::function<YAML::Node(rttr::variant const&, YamlSerializeContext const&)> const& extension_func = {}
) noexcept -> YAML::Node;

[[nodiscard]] SORCERYAPI
auto ReflectionSerializeToYaml(
  rttr::variant const& v,
  YamlSerializeContext const& ctx,
  std::function<YAML::Node(rttr::variant const&, YamlSerializeContext const&)> const& extension_func = {}
) noexcept -> YAML::Node;

// Reflection-based deserialization from YAML

template<typename T> requires (!std::derived_from<T, Object>)
auto ReflectionDeserializeFromYaml(
  YAML::Node const& node,
  T& obj,
  YamlDeserializeContext const& ctx,
  std::function<void(YAML::Node const&, rttr::variant&, YamlDeserializeContext const&)> const& extension_func = {}
) noexcept -> void;

SORCERYAPI
auto ReflectionDeserializeFromYaml(
  YAML::Node const& node,
  Object& obj,
  YamlDeserializeContext const& ctx,
  std::function<void(YAML::Node const&, rttr::variant&, YamlDeserializeContext const&)> const& extension_func = {}
) noexcept -> void;

SORCERYAPI
auto ReflectionDeserializeFromYaml(
  YAML::Node const& node,
  rttr::variant& v,
  YamlDeserializeContext const& ctx,
  std::function<void(YAML::Node const&, rttr::variant&, YamlDeserializeContext const&)> const& extension_func = {}
) noexcept -> void;

SORCERYAPI
auto ReflectionDeserializeFromYaml(
  YAML::Node const& node,
  rttr::variant&& v,
  YamlDeserializeContext const& ctx,
  std::function<void(YAML::Node const&, rttr::variant&, YamlDeserializeContext const&)> const& extension_func = {}
) noexcept -> void;

// Serialization to binary

template<typename T>
  requires std::is_integral_v<T>
           || (std::is_floating_point_v<T> && std::numeric_limits<T>::is_iec559)
           || std::is_enum_v<T>
auto SerializeToBinary(
  T val,
  std::vector<std::byte>& bytes
) noexcept -> void;

SORCERYAPI
auto SerializeToBinary(
  std::string_view sv,
  std::vector<std::byte>& bytes
) noexcept -> void;

// Deserialization from binary

template<typename T>
  requires std::is_integral_v<T>
           || (std::is_floating_point_v<T> && std::numeric_limits<T>::is_iec559)
           || std::is_enum_v<T>
[[nodiscard]]
auto DeserializeFromBinary(
  std::span<std::byte const> bytes,
  T& val
) noexcept -> bool;

[[nodiscard]] SORCERYAPI
auto DeserializeFromBinary(
  std::span<std::byte const> bytes,
  std::string& str
) noexcept -> bool;
}


#include "serialization.inl"
