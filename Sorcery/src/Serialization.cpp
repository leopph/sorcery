#include "Serialization.hpp"

#include "Resources/Resource.hpp"
#include "ResourceManager.hpp"


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

  if (auto const underlyingType{
    v.get_type().is_wrapper()
      ? v.get_type().get_wrapped_type()
      : v.get_type()
  }; underlyingType.is_arithmetic()) {
    [&v, &underlyingType, &retNode]<typename... Types> {
      ([&v, &underlyingType, &retNode]<typename T> {
        if (underlyingType == rttr::type::get<T>()) {
          retNode = v.get_value<T>();
        }
      }.template operator()<Types>(), ...);
    }.operator()<char, signed char, unsigned char, short, unsigned short, int, unsigned, long, unsigned long, long
                 long, unsigned long long, float, double, long double>();
  } else if (v.is_sequential_container()) {
    for (auto const& elem : v.create_sequential_view()) {
      retNode.push_back(ReflectionSerializeToYaml(elem, extensionFunc));
    }
  } else if (v.get_type().is_wrapper()) {
    // We presume that it's a std::reference_wrapper then
    if (!v.get_type().get_wrapped_type().is_pointer()) {
      // TODO
    }
  } else if (v.get_type().is_pointer()) {
    if (v.get_type().get_raw_type().is_derived_from(rttr::type::get<Resource>())) {
      retNode = v.get_value<Resource*>()->GetGuid();
    }
  } else if (underlyingType.is_class()) {
    for (auto const prop : underlyingType.get_properties()) {
      retNode[prop.get_name().to_string()] = ReflectionSerializeToYaml(prop.get_value(v), extensionFunc);
    }
  } else if (extensionFunc) {
    retNode = extensionFunc(v);
  }

  return retNode;
}


auto ReflectionDeserializeFromYaml(YAML::Node const& objNode, rttr::variant& v, std::function<void(YAML::Node const&, rttr::variant&)> const& extensionFunc) -> void {
  if (auto const underlyingType{
    v.get_type().is_wrapper()
      ? v.get_type().get_wrapped_type()
      : v.get_type()
  }; underlyingType.is_arithmetic()) {
    [&v, &underlyingType, &objNode]<typename... Types> {
      ([&v, &underlyingType, &objNode]<typename T> {
        if (underlyingType == rttr::type::get<T>()) {
          v = objNode.as<T>();
        }
      }.template operator()<Types>(), ...);
    }.operator()<char, signed char, unsigned char, short, unsigned short, int, unsigned, long, unsigned long, long
                 long, unsigned long long, float, double, long double>();
  } else if (v.is_sequential_container()) {
    auto seqView{ v.create_sequential_view() };
    seqView.set_size(objNode.size());

    for (auto i{ 0ll }; i < std::ssize(objNode); i++) {
      rttr::variant elem;
      ReflectionDeserializeFromYaml(objNode[i], elem, extensionFunc);
      seqView.set_value(i, elem);
    }
  } else if (v.get_type().is_wrapper()) {
    // We presume that it's a std::reference_wrapper then
    if (!v.get_type().get_wrapped_type().is_pointer()) {
      // TODO
    }
  } else if (v.get_type().is_pointer()) {
    if (v.get_type().get_raw_type().is_derived_from(rttr::type::get<Resource>())) {
      v = gResourceManager.LoadResource(objNode.as<Guid>()).Get();
    }
  } else if (underlyingType.is_class()) {
    for (auto const prop : underlyingType.get_properties()) {
      auto propValue{ prop.get_value(v) };
      ReflectionDeserializeFromYaml(objNode[prop.get_name().to_string()], propValue, extensionFunc);
    }
  } else if (extensionFunc) {
    extensionFunc(objNode, v);
  }
}
}
