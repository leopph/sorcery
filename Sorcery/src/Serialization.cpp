#include "Serialization.hpp"

#include "Resources/Resource.hpp"
#include "ResourceManager.hpp"

#include <cassert>


namespace YAML {
auto convert<sorcery::Quaternion>::encode(sorcery::Quaternion const& q) -> Node {
  Node node;
  node.SetStyle(EmitterStyle::Flow);
  node.push_back(q.w);
  node.push_back(q.x);
  node.push_back(q.y);
  node.push_back(q.z);
  return node;
}


auto convert<sorcery::Quaternion>::decode(Node const& node, sorcery::Quaternion& q) -> bool {
  if (!node.IsSequence() || node.size() != 4) {
    return false;
  }
  q.w = node[0].as<sorcery::f32>();
  q.x = node[1].as<sorcery::f32>();
  q.y = node[2].as<sorcery::f32>();
  q.z = node[3].as<sorcery::f32>();
  return true;
}


auto convert<sorcery::Guid>::encode(sorcery::Guid const& guid) -> Node {
  return Node{ static_cast<std::string>(guid) };
}


auto convert<sorcery::Guid>::decode(Node const& node, sorcery::Guid& guid) -> bool {
  if (!node.IsScalar()) {
    return false;
  }
  guid = sorcery::Guid::Parse(node.as<std::string>());
  return true;
}
}


namespace sorcery {
auto ReflectionSerializeToYaml(Object const& obj, std::function<YAML::Node(rttr::variant const&)> const& extensionFunc) -> YAML::Node {
  YAML::Node ret;
  for (auto const& prop : rttr::type::get(obj).get_properties()) {
    ret[prop.get_name().to_string()] = ReflectionSerializeToYaml(prop.get_value(obj), extensionFunc);
  }
  return ret;
}


auto ReflectionSerializeToYaml(rttr::variant const& v, std::function<YAML::Node(rttr::variant const&)> const& extensionFunc) -> YAML::Node {
  if (v.get_type() == rttr::type::get<bool>()) {
    return YAML::Node{ v.get_value<bool>() };
  }

  if (v.get_type() == rttr::type::get<char>()) {
    return YAML::Node{ v.get_value<char>() };
  }

  if (v.get_type() == rttr::type::get<signed char>()) {
    return YAML::Node{ v.get_value<signed char>() };
  }

  if (v.get_type() == rttr::type::get<unsigned char>()) {
    return YAML::Node{ v.get_value<unsigned char>() };
  }

  if (v.get_type() == rttr::type::get<short>()) {
    return YAML::Node{ v.get_value<short>() };
  }

  if (v.get_type() == rttr::type::get<unsigned short>()) {
    return YAML::Node{ v.get_value<unsigned short>() };
  }

  if (v.get_type() == rttr::type::get<int>()) {
    return YAML::Node{ v.get_value<int>() };
  }

  if (v.get_type() == rttr::type::get<unsigned>()) {
    return YAML::Node{ v.get_value<unsigned>() };
  }

  if (v.get_type() == rttr::type::get<long>()) {
    return YAML::Node{ v.get_value<long>() };
  }

  if (v.get_type() == rttr::type::get<unsigned long>()) {
    return YAML::Node{ v.get_value<unsigned long>() };
  }

  if (v.get_type() == rttr::type::get<long long>()) {
    return YAML::Node{ v.get_value<long long>() };
  }

  if (v.get_type() == rttr::type::get<unsigned long long>()) {
    return YAML::Node{ v.get_value<unsigned long long>() };
  }

  if (v.get_type() == rttr::type::get<float>()) {
    return YAML::Node{ v.get_value<float>() };
  }

  if (v.get_type() == rttr::type::get<double>()) {
    return YAML::Node{ v.get_value<double>() };
  }

  if (v.get_type() == rttr::type::get<long double>()) {
    return YAML::Node{ v.get_value<long double>() };
  }

  if (v.get_type() == rttr::type::get<std::string>()) {
    return YAML::Node{ v.get_value<std::string>() };
  }

  if (v.get_type().is_enumeration()) {
    auto const enumeration{ v.get_type().get_enumeration() };
    auto underlying{ v };
    auto const success{ underlying.convert(enumeration.get_underlying_type()) };
    assert(success);
    return ReflectionSerializeToYaml(underlying, extensionFunc);
  }

  if (v.get_type().is_pointer() && v.get_type().get_raw_type().is_derived_from(rttr::type::get<Resource>())) {
    return YAML::Node{ v.get_value<ObserverPtr<Resource>>()->GetGuid() };
  }

  if (v.is_sequential_container()) {
    auto const container{ v.create_sequential_view() };
    assert(container.is_valid());

    YAML::Node node;

    for (auto const& elem : container) {
      auto const value{ elem.extract_wrapped_value() };
      assert(value.is_valid());
      node.push_back(ReflectionSerializeToYaml(value, extensionFunc));
    }

    return node;
  }

  if (v.is_associative_container()) {
    return {};
  }

  if (v.get_type().is_enumeration()) {
    return {}; // TODO
  }

  if (v.get_type().is_class()) {
    YAML::Node node;

    for (auto const& prop : v.get_type().get_properties()) {
      auto const value{ prop.get_value(v) };
      assert(value.is_valid());
      node[prop.get_name().to_string()] = ReflectionSerializeToYaml(value, extensionFunc);
    }

    return node;
  }

  if (v.get_type().is_wrapper() && v.get_type().get_wrapped_type().is_class()) {
    YAML::Node node;

    for (auto const& prop : v.get_type().get_wrapped_type().get_properties()) {
      auto const value{ prop.get_value(v) };
      assert(value.is_valid());
      node[prop.get_name().to_string()] = ReflectionSerializeToYaml(value, extensionFunc);
    }

    return node;
  }

  if (extensionFunc) {
    return extensionFunc(v);
  }

  return {};
}


auto ReflectionDeserializeFromYaml(YAML::Node const& node, Object& obj, std::function<void(YAML::Node const&, rttr::variant&)> const& extensionFunc) -> void {
  for (auto const& prop : rttr::type::get(obj).get_properties()) {
    auto value{ prop.get_value(obj) };
    assert(value.is_valid());
    ReflectionDeserializeFromYaml(node[prop.get_name().to_string()], value, extensionFunc);
    assert(value.is_valid());
    auto const success{ prop.set_value(obj, value) };
    assert(success);
  }
}


auto ReflectionDeserializeFromYaml(YAML::Node const& node, rttr::variant& v, std::function<void(YAML::Node const&, rttr::variant&)> const& extensionFunc) -> void {
  if (node.IsNull() || !v.get_type().is_valid()) {
    return;
  }

  if (v.get_type() == rttr::type::get<bool>()) {
    try {
      v = node.as<bool>();
    } catch (...) {}
    return;
  }

  if (v.get_type() == rttr::type::get<char>()) {
    try {
      v = node.as<char>();
    } catch (...) { }
    return;
  }

  if (v.get_type() == rttr::type::get<signed char>()) {
    try {
      v = node.as<signed char>();
    } catch (...) { }
    return;
  }

  if (v.get_type() == rttr::type::get<unsigned char>()) {
    try {
      v = node.as<unsigned char>();
    } catch (...) { }
    return;
  }

  if (v.get_type() == rttr::type::get<short>()) {
    try {
      v = node.as<short>();
    } catch (...) { }
    return;
  }

  if (v.get_type() == rttr::type::get<unsigned short>()) {
    try {
      v = node.as<unsigned short>();
    } catch (...) { }
    return;
  }

  if (v.get_type() == rttr::type::get<int>()) {
    try {
      v = node.as<int>();
    } catch (...) { }
    return;
  }

  if (v.get_type() == rttr::type::get<unsigned>()) {
    try {
      v = node.as<unsigned>();
    } catch (...) { }
    return;
  }

  if (v.get_type() == rttr::type::get<long>()) {
    try {
      v = node.as<long>();
    } catch (...) { }
    return;
  }

  if (v.get_type() == rttr::type::get<unsigned long>()) {
    try {
      v = node.as<unsigned long>();
    } catch (...) { }
    return;
  }

  if (v.get_type() == rttr::type::get<long long>()) {
    try {
      v = node.as<long long>();
    } catch (...) { }
    return;
  }

  if (v.get_type() == rttr::type::get<unsigned long long>()) {
    try {
      v = node.as<unsigned long long>();
    } catch (...) { }
    return;
  }

  if (v.get_type() == rttr::type::get<float>()) {
    try {
      v = node.as<float>();
    } catch (...) { }
    return;
  }

  if (v.get_type() == rttr::type::get<double>()) {
    try {
      v = node.as<double>();
    } catch (...) { }
    return;
  }

  if (v.get_type() == rttr::type::get<long double>()) {
    try {
      v = node.as<long double>();
    } catch (...) { }
    return;
  }

  if (v.get_type() == rttr::type::get<std::string>()) {
    try {
      v = node.as<std::string>();
    } catch (...) { }
    return;
  }

  if (v.get_type().is_enumeration()) {
    auto const enumeration{ v.get_type().get_enumeration() };
    assert(enumeration.is_valid());
    auto copy{ v };
    assert(copy.is_valid());
    auto success{ copy.convert(enumeration.get_underlying_type()) };
    assert(success);
    ReflectionDeserializeFromYaml(node, copy, extensionFunc);
    assert(copy.is_valid());
    success = copy.convert(enumeration.get_type());
    assert(success);
    v = copy;
    return;
  }

  if (v.get_type().is_pointer() && v.get_type().get_raw_type().is_derived_from(rttr::type::get<Resource>())) {
    try {
      auto const type{ v.get_type() };
      v = gResourceManager.LoadResource(node.as<Guid>());
      auto const success{ v.convert(type) };
      assert(success);
    } catch (...) { }
    return;
  }

  if (v.is_sequential_container()) {
    auto container{ v.create_sequential_view() };
    assert(container.is_valid());

    if (container.is_dynamic()) {
      auto const success{ container.set_size(node.size()) };
      assert(success);
    }

    for (std::size_t i{ 0 }; i < node.size() && container.get_size(); i++) {
      auto value{ container.get_value(i).extract_wrapped_value() };
      assert(value.is_valid());
      ReflectionDeserializeFromYaml(node[i], value, extensionFunc);
      assert(value.is_valid());
      auto const success{ container.set_value(i, value) };
      assert(success);
    }

    return;
  }

  if (v.is_associative_container()) {
    return;
  }

  if (v.get_type().is_enumeration()) {
    return; // TODO
  }

  if (v.get_type().is_class()) {
    for (auto const& prop : v.get_type().get_properties()) {
      auto value{ prop.get_value(v) };
      assert(value.is_valid());
      ReflectionDeserializeFromYaml(node[prop.get_name().to_string()], value, extensionFunc);
      assert(value.is_valid());
      auto const success{ prop.set_value(v, value) };
      assert(success);
    }

    return;
  }

  if (v.get_type().is_wrapper() && v.get_type().get_wrapped_type().is_class()) {
    for (auto const& prop : v.get_type().get_wrapped_type().get_properties()) {
      auto value{ prop.get_value(v) };
      assert(value.is_valid());
      ReflectionDeserializeFromYaml(node[prop.get_name().to_string()], value, extensionFunc);
      assert(value.is_valid());
      auto const success{ prop.set_value(v, value) };
      assert(success);
    }

    return;
  }

  if (extensionFunc) {
    extensionFunc(node, v);
  }
}


auto ReflectionDeserializeFromYaml(YAML::Node const& node, rttr::variant&& v, std::function<void(YAML::Node const&, rttr::variant&)> const& extensionFunc) -> void {
  ReflectionDeserializeFromYaml(node, v, extensionFunc);
}
}
