#include "Serialization.hpp"


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
auto ReflectionSerializeToYAML(rttr::variant const& obj, std::function<YAML::Node(rttr::variant const&)> const& extensionFunc) -> YAML::Node {
  YAML::Node retNode;

  for (auto const prop : obj.get_type().get_properties()) {
    retNode["properties"].push_back(detail::ReflectionSerializeToYAML(prop.get_value(obj), extensionFunc));
  }

  return retNode;
}


auto ReflectionDeserializeFromYAML(YAML::Node const& objNode, rttr::variant& obj, std::function<void(YAML::Node const&, rttr::variant&)> const& extensionFunc) -> void {
  for (auto const prop : obj.get_type().get_properties()) {
    rttr::variant propValue{ prop.get_value(obj) };
    detail::ReflectionDeserializeFromYAML(objNode["properties"], propValue, extensionFunc);
    std::ignore = prop.set_value(obj, propValue);
  }
}


namespace detail {
auto ReflectionSerializeToYAML(rttr::variant const& variant, std::function<YAML::Node(rttr::variant const&)> const& extensionFunc) -> YAML::Node {
  auto const objType{ variant.get_type() };
  auto const isWrapper{ objType.is_wrapper() };
  auto const wrappedType{ objType.get_wrapped_type() };
  auto const rawWrappedType{
    isWrapper
      ? wrappedType.get_raw_type()
      : objType.get_raw_type()
  };

  YAML::Node ret;

  if (rawWrappedType.is_arithmetic()) {
    if (rawWrappedType == rttr::type::get<char>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<char>()
              : variant.get_value<char>();
    } else if (rawWrappedType == rttr::type::get<signed char>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<signed char>()
              : variant.get_value<signed char>();
    } else if (rawWrappedType == rttr::type::get<unsigned char>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<unsigned char>()
              : variant.get_value<unsigned char>();
    } else if (rawWrappedType == rttr::type::get<short>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<short>()
              : variant.get_value<short>();
    } else if (rawWrappedType == rttr::type::get<unsigned short>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<unsigned short>()
              : variant.get_value<unsigned short>();
    } else if (rawWrappedType == rttr::type::get<int>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<int>()
              : variant.get_value<int>();
    } else if (rawWrappedType == rttr::type::get<unsigned>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<unsigned>()
              : variant.get_value<unsigned>();
    } else if (rawWrappedType == rttr::type::get<long>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<long>()
              : variant.get_value<long>();
    } else if (rawWrappedType == rttr::type::get<unsigned long>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<unsigned long>()
              : variant.get_value<unsigned long>();
    } else if (rawWrappedType == rttr::type::get<long long>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<long long>()
              : variant.get_value<long long>();
    } else if (rawWrappedType == rttr::type::get<unsigned long long>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<unsigned long long>()
              : variant.get_value<unsigned long long>();
    } else if (rawWrappedType == rttr::type::get<float>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<float>()
              : variant.get_value<float>();
    } else if (rawWrappedType == rttr::type::get<double>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<double>()
              : variant.get_value<double>();
    } else if (rawWrappedType == rttr::type::get<long double>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<long double>()
              : variant.get_value<long double>();
    }
  } else if (extensionFunc) {
    ret = extensionFunc(variant);
  }

  return ret;
}


auto ReflectionDeserializeFromYAML(YAML::Node const& objNode, rttr::variant& variant, std::function<void(YAML::Node const&, rttr::variant&)> const& extensionFunc) -> void {
  auto const objType{ variant.get_type() };
  auto const isWrapper{ objType.is_wrapper() };
  auto const wrappedType{ objType.get_wrapped_type() };
  auto const rawWrappedType{
    isWrapper
      ? wrappedType.get_raw_type()
      : objType.get_raw_type()
  };

  if (rawWrappedType.is_arithmetic()) {
    if (rawWrappedType == rttr::type::get<char>()) {
      variant = objNode.as<char>();
    } else if (rawWrappedType == rttr::type::get<signed char>()) {
      variant = objNode.as<signed char>();
    } else if (rawWrappedType == rttr::type::get<unsigned char>()) {
      variant = objNode.as<unsigned char>();
    } else if (rawWrappedType == rttr::type::get<short>()) {
      variant = objNode.as<short>();
    } else if (rawWrappedType == rttr::type::get<unsigned short>()) {
      variant = objNode.as<unsigned short>();
    } else if (rawWrappedType == rttr::type::get<int>()) {
      variant = objNode.as<int>();
    } else if (rawWrappedType == rttr::type::get<unsigned>()) {
      variant = objNode.as<unsigned>();
    } else if (rawWrappedType == rttr::type::get<long>()) {
      variant = objNode.as<long>();
    } else if (rawWrappedType == rttr::type::get<unsigned long>()) {
      variant = objNode.as<unsigned long>();
    } else if (rawWrappedType == rttr::type::get<long long>()) {
      variant = objNode.as<long long>();
    } else if (rawWrappedType == rttr::type::get<unsigned long long>()) {
      variant = objNode.as<unsigned long long>();
    } else if (rawWrappedType == rttr::type::get<float>()) {
      variant = objNode.as<float>();
    } else if (rawWrappedType == rttr::type::get<double>()) {
      variant = objNode.as<double>();
    } else if (rawWrappedType == rttr::type::get<long double>()) {
      variant = objNode.as<long double>();
    }
  } else if (extensionFunc) {
    extensionFunc(objNode, variant);
  }
}
}
}
