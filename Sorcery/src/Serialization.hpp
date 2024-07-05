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
                                std::is_floating_point_v<T> && std::numeric_limits<T>::is_iec559()) || std::is_enum_v<T>
[[nodiscard]] auto DeserializeFromBinary(std::span<std::byte const> bytes, T& val) noexcept -> bool;
[[nodiscard]] LEOPPHAPI auto DeserializeFromBinary(std::span<std::byte const> bytes, std::string& str) noexcept -> bool;
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
template<typename T> requires (!std::derived_from<T, Object>)
auto ReflectionSerializeToYaml(T const& obj,
                               std::function<YAML::Node(rttr::variant const&)> const& extensionFunc) noexcept ->
  YAML::Node {
  if constexpr (std::is_arithmetic_v<T> || std::is_enum_v<T>) {
    return ReflectionSerializeToYaml(rttr::variant{obj}, extensionFunc);
  } else {
    auto const type{rttr::type::get(obj)};
    assert(type.is_valid());
    if (type.is_sequential_container()) {
      return ReflectionSerializeToYaml(rttr::variant{std::ref(obj)}, extensionFunc);
    }

    if (type.is_associative_container() || type.is_pointer() || type.is_wrapper()) {
      return {};
    }

    if (type.is_class()) {
      YAML::Node ret;
      for (auto const& prop : type.get_properties()) {
        auto propValue{prop.get_value(obj)};
        assert(propValue.is_valid());
        ret[prop.get_name().to_string()] = ReflectionSerializeToYaml(propValue, extensionFunc);
      }
      return ret;
    }

    return {};
  }
}


template<typename T> requires (!std::derived_from<T, Object>)
auto ReflectionDeserializeFromYaml(YAML::Node const& node, T& obj,
                                   std::function<void(YAML::Node const&, rttr::variant&)> const& extensionFunc) noexcept
  -> void {
  if constexpr (std::is_arithmetic_v<T> || std::is_enum_v<T>) {
    rttr::variant v{obj};
    assert(v.is_valid());
    ReflectionDeserializeFromYaml(node, v, extensionFunc);
    assert(v.is_valid());
    obj = v.get_value<T>();
  } else {
    auto const type{rttr::type::get(obj)};
    assert(type.is_valid());

    if (type.is_sequential_container()) {
      ReflectionDeserializeFromYaml(node, rttr::variant{std::ref(obj)}, extensionFunc);
      return;
    }

    if (type.is_associative_container() || type.is_pointer() || type.is_wrapper()) {
      return;
    }

    if (type.is_class()) {
      for (auto const& prop : type.get_properties()) {
        auto propValue{prop.get_value(obj)};
        assert(propValue.is_valid());
        ReflectionDeserializeFromYaml(node[prop.get_name().to_string()], propValue, extensionFunc);
        assert(propValue.is_valid());
        [[maybe_unused]] auto const success{prop.set_value(obj, propValue)};
        assert(success);
      }
    }
  }
}


template<typename T> requires std::is_integral_v<T> || (
                                std::is_floating_point_v<T> && std::numeric_limits<T>::is_iec559) || std::is_enum_v<T>
auto SerializeToBinary(T const val, std::vector<std::byte>& bytes) noexcept -> void {
  std::ranges::copy_n(reinterpret_cast<std::byte const*>(&val), sizeof(val), std::back_inserter(bytes));
}


template<typename T> requires std::is_integral_v<T> || (
                                std::is_floating_point_v<T> && std::numeric_limits<T>::is_iec559()) || std::is_enum_v<T>
auto DeserializeFromBinary(std::span<std::byte const> bytes, T& val) noexcept -> bool {
  if (sizeof(T) > std::size(bytes)) {
    return false;
  }

  std::ranges::copy_n(std::begin(bytes), sizeof(val), reinterpret_cast<std::byte*>(&val));
  return true;
}
}
