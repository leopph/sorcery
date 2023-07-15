#include "ReflectionSerialization.hpp"


namespace sorcery {
auto ReflectionSerializeToYAML(rttr::variant const& variant) -> YAML::Node {
  auto const objType{ variant.get_type() };
  auto const isWrapper{ objType.is_wrapper() };
  auto const rawWrappedType{
    objType.is_wrapper()
      ? objType.get_wrapped_type().get_raw_type()
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
  } else if (rawWrappedType.is_sequential_container()) {
    for (auto const& elem : variant.create_sequential_view()) {
      ret.push_back(ReflectionSerializeToYAML(elem));
    }
  } else if (rawWrappedType.is_class()) {
    for (auto const& prop : objType.get_properties()) {
      ret[prop.get_name().to_string()] = ReflectionSerializeToYAML(prop.get_value(variant));
    }
  }

  return ret;
}
}
