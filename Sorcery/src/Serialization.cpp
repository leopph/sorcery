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
auto ReflectionSerializeToYaml(rttr::variant const& v, std::function<YAML::Node(rttr::variant const&)> const& extensionFunc) -> YAML::Node {
  YAML::Node retNode;

  if (auto const typeOrWrappedType{
    (v.get_type().is_wrapper()
       ? v.get_type().get_wrapped_type()
       : v.get_type())
  }; typeOrWrappedType.is_arithmetic()) {
    [&v, &retNode, &typeOrWrappedType]<typename... Types> {
      ([&v, &retNode, &typeOrWrappedType]<typename T> {
        if (typeOrWrappedType == rttr::type::get<T>()) {
          retNode = v.get_type().is_wrapper()
                      ? v.get_wrapped_value<T>()
                      : v.get_value<T>();
        }
      }.template operator()<Types>(), ...);
    }.operator()<char, signed char, unsigned char, short, unsigned short, int, unsigned, long, unsigned long, long
                 long, unsigned long long, float, double, long double>();
  } else if (v.can_convert<std::string>()) {
    auto success{ true };
    auto const str{ v.convert<std::string>(&success) };
    if (success) {
      retNode = str;
    }
  } else if (v.is_sequential_container()) {
    for (auto const& elem : v.create_sequential_view()) {
      retNode.push_back(ReflectionSerializeToYaml(elem, extensionFunc));
    }
  } else if (typeOrWrappedType.is_pointer() && typeOrWrappedType.get_raw_type().is_derived_from(rttr::type::get<Resource>())) {
    retNode = (v.get_type().is_wrapper()
                 ? v.get_wrapped_value<Resource*>()
                 : v.get_value<Resource*>())->GetGuid();
  } else if (typeOrWrappedType.is_class()) {
    for (auto const& prop : typeOrWrappedType.get_properties()) {
      retNode[prop.get_name().to_string()] = ReflectionSerializeToYaml(prop.get_value(v), extensionFunc);
    }
  } else if (extensionFunc) {
    retNode = extensionFunc(v);
  }

  return retNode;
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

  if (v.get_type() == rttr::type::get<std::string>()) {
    try {
      v = node.as<std::string>();
    } catch (...) { }
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
    return;
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
}
