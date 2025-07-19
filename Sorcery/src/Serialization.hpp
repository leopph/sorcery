#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstdint>
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

#include "resource_id.hpp"
#include "Core.hpp"
#include "Math.hpp"
#include "Guid.hpp"
#include "Reflection.hpp"
#include "Object.hpp"


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


template<>
struct convert<sorcery::ResourceId> {
  LEOPPHAPI static auto encode(sorcery::ResourceId const& res_id) -> Node;
  LEOPPHAPI static auto decode(Node const& node, sorcery::ResourceId& res_id) -> bool;
};
}


namespace sorcery {
template<typename T> requires (!std::derived_from<T, Object>)
[[nodiscard]] auto ReflectionSerializeToYaml(T const& obj,
                                             std::function<YAML::Node(rttr::variant const&)> const& extensionFunc = {})
  noexcept -> YAML::Node;
[[nodiscard]] LEOPPHAPI auto ReflectionSerializeToYaml(Object const& obj,
                                                       std::function<YAML::Node(rttr::variant const&)> const&
                                                         extensionFunc = {}) noexcept -> YAML::Node;
[[nodiscard]] LEOPPHAPI auto ReflectionSerializeToYaml(rttr::variant const& v,
                                                       std::function<YAML::Node(rttr::variant const&)> const&
                                                         extensionFunc = {}) noexcept -> YAML::Node;

template<typename T> requires (!std::derived_from<T, Object>)
auto ReflectionDeserializeFromYaml(YAML::Node const& node, T& obj,
                                   std::function<void(YAML::Node const&, rttr::variant&)> const& extensionFunc = {})
  noexcept -> void;
LEOPPHAPI auto ReflectionDeserializeFromYaml(YAML::Node const& node, Object& obj,
                                             std::function<void(YAML::Node const&, rttr::variant&)> const& extensionFunc
                                               = {}) noexcept -> void;
LEOPPHAPI auto ReflectionDeserializeFromYaml(YAML::Node const& node, rttr::variant& v,
                                             std::function<void(YAML::Node const&, rttr::variant&)> const& extensionFunc
                                               = {}) noexcept -> void;
LEOPPHAPI auto ReflectionDeserializeFromYaml(YAML::Node const& node, rttr::variant&& v,
                                             std::function<void(YAML::Node const&, rttr::variant&)> const& extensionFunc
                                               = {}) noexcept -> void;

template<typename T> requires std::is_integral_v<T> || (
                                std::is_floating_point_v<T> && std::numeric_limits<T>::is_iec559) || std::is_enum_v<T>
auto SerializeToBinary(T val, std::vector<std::byte>& bytes) noexcept -> void;
LEOPPHAPI auto SerializeToBinary(std::string_view sv, std::vector<std::byte>& bytes) noexcept -> void;

template<typename T> requires std::is_integral_v<T> || (
                                std::is_floating_point_v<T> && std::numeric_limits<T>::is_iec559) || std::is_enum_v<T>
[[nodiscard]] auto DeserializeFromBinary(std::span<std::byte const> bytes, T& val) noexcept -> bool;
[[nodiscard]] LEOPPHAPI auto DeserializeFromBinary(std::span<std::byte const> bytes, std::string& str) noexcept -> bool;
}


#include "serialization.inl"
