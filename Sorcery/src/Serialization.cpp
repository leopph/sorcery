#include "Serialization.hpp"

#include "app.hpp"
#include "resource_manager.hpp"
#include "resource_reference.hpp"
#include "Resources/Resource.hpp"

#include <cassert>

static_assert(
  std::endian::native == std::endian::little &&
  "Serialization and deserialization is only supported on little endian architectures!");


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
  return Node{static_cast<std::string>(guid)};
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
auto ReflectionSerializeToYaml(
  Object const& obj,
  YamlSerializeContext const& ctx,
  std::function<YAML::Node(rttr::variant const&, YamlSerializeContext const&)> const& extension_func
) noexcept -> YAML::Node {
  YAML::Node ret;
  for (auto const& prop : rttr::type::get(obj).get_properties()) {
    ret[prop.get_name().to_string()] = ReflectionSerializeToYaml(prop.get_value(obj), ctx, extension_func);
  }
  return ret;
}


auto ReflectionSerializeToYaml(
  rttr::variant const& v,
  YamlSerializeContext const& ctx,
  std::function<YAML::Node(rttr::variant const&, YamlSerializeContext const&)> const& extension_func
) noexcept -> YAML::Node {
  auto const variant_type{v.get_type()};

  if (variant_type == rttr::type::get<bool>()) {
    return YAML::Node{v.get_value<bool>()};
  }

  if (variant_type == rttr::type::get<char>()) {
    return YAML::Node{v.get_value<char>()};
  }

  if (variant_type == rttr::type::get<signed char>()) {
    return YAML::Node{v.get_value<signed char>()};
  }

  if (variant_type == rttr::type::get<unsigned char>()) {
    return YAML::Node{v.get_value<unsigned char>()};
  }

  if (variant_type == rttr::type::get<short>()) {
    return YAML::Node{v.get_value<short>()};
  }

  if (variant_type == rttr::type::get<unsigned short>()) {
    return YAML::Node{v.get_value<unsigned short>()};
  }

  if (variant_type == rttr::type::get<int>()) {
    return YAML::Node{v.get_value<int>()};
  }

  if (variant_type == rttr::type::get<unsigned>()) {
    return YAML::Node{v.get_value<unsigned>()};
  }

  if (variant_type == rttr::type::get<long>()) {
    return YAML::Node{v.get_value<long>()};
  }

  if (variant_type == rttr::type::get<unsigned long>()) {
    return YAML::Node{v.get_value<unsigned long>()};
  }

  if (variant_type == rttr::type::get<long long>()) {
    return YAML::Node{v.get_value<long long>()};
  }

  if (variant_type == rttr::type::get<unsigned long long>()) {
    return YAML::Node{v.get_value<unsigned long long>()};
  }

  if (variant_type == rttr::type::get<float>()) {
    return YAML::Node{v.get_value<float>()};
  }

  if (variant_type == rttr::type::get<double>()) {
    return YAML::Node{v.get_value<double>()};
  }

  if (variant_type == rttr::type::get<long double>()) {
    return YAML::Node{v.get_value<long double>()};
  }

  if (variant_type == rttr::type::get<std::string>()) {
    return YAML::Node{v.get_value<std::string>()};
  }

  if (variant_type.is_enumeration()) {
    auto const enumeration{variant_type.get_enumeration()};
    auto underlying{v};
    [[maybe_unused]] auto const success{underlying.convert(enumeration.get_underlying_type())};
    assert(success);
    return ReflectionSerializeToYaml(underlying, ctx, extension_func);
  }

  // T*, ObserverPtr<T>, etc.
  if (variant_type.is_pointer() || (variant_type.is_wrapper() && variant_type.get_wrapped_type().is_pointer())) {
    // std::remove_pointer<T>
    auto const ptr_type{variant_type.is_pointer() ? variant_type : variant_type.get_wrapped_type()};

    // std::remove_cv<T>
    auto const raw_type{ptr_type.get_raw_type()};

    // std::derived_from<T, Resource>
    if (raw_type.is_derived_from(rttr::type::get<Resource>())) {
      // if the original type is a wrapper, we extract the pointer here
      auto const res{(variant_type.is_pointer() ? v : v.extract_wrapped_value()).get_value<Resource*>()};
      auto const res_id{res ? res->GetResId() : ResourceId::Invalid()};

      switch (ctx.resource_ref_serialization) {
        case ResourceRefSerialization::kGlobal:
          return SerializeGlobalResourceId(res_id);
        case ResourceRefSerialization::kLocal:
          return SerializeLocalResourceId(res_id.GetIdxInFile());
      }
    }
  }

  if (v.is_sequential_container()) {
    auto const container{v.create_sequential_view()};
    assert(container.is_valid());

    YAML::Node node;

    for (auto const& elem : container) {
      auto const value{elem.extract_wrapped_value()};
      assert(value.is_valid());
      node.push_back(ReflectionSerializeToYaml(value, ctx, extension_func));
    }

    return node;
  }

  if (v.is_associative_container()) {
    return {};
  }

  if (variant_type.is_class()) {
    YAML::Node node;

    for (auto const& prop : variant_type.get_properties()) {
      auto const value{prop.get_value(v)};
      assert(value.is_valid());
      node[prop.get_name().to_string()] = ReflectionSerializeToYaml(value, ctx, extension_func);
    }

    return node;
  }

  if (variant_type.is_wrapper() && variant_type.get_wrapped_type().is_class()) {
    YAML::Node node;

    for (auto const& prop : variant_type.get_wrapped_type().get_properties()) {
      auto const value{prop.get_value(v)};
      assert(value.is_valid());
      node[prop.get_name().to_string()] = ReflectionSerializeToYaml(value, ctx, extension_func);
    }

    return node;
  }

  if (extension_func) {
    return extension_func(v, ctx);
  }

  return {};
}


auto ReflectionDeserializeFromYaml(
  YAML::Node const& node,
  Object& obj,
  YamlDeserializeContext const& ctx,
  std::function<void(YAML::Node const&, rttr::variant&, YamlDeserializeContext const&)> const& extension_func
) noexcept -> void {
  for (auto const& prop : rttr::type::get(obj).get_properties()) {
    auto value{prop.get_value(obj)};
    assert(value.is_valid());
    ReflectionDeserializeFromYaml(node[prop.get_name().to_string()], value, ctx, extension_func);
    assert(value.is_valid());
    [[maybe_unused]] auto const success{prop.set_value(obj, value)};
    assert(success);
  }
}


auto ReflectionDeserializeFromYaml(
  YAML::Node const& node,
  rttr::variant& v,
  YamlDeserializeContext const& ctx,
  std::function<void(YAML::Node const&, rttr::variant&, YamlDeserializeContext const&)> const& extension_func
) noexcept -> void {
  if (!node.IsDefined() || node.IsNull()) {
    return;
  }

  auto const variant_type{v.get_type()};

  if (!variant_type.is_valid()) {
    return;
  }

  if (variant_type == rttr::type::get<bool>()) {
    try {
      v = node.as<bool>();
    } catch (...) {}
    return;
  }

  if (variant_type == rttr::type::get<char>()) {
    try {
      v = node.as<char>();
    } catch (...) {}
    return;
  }

  if (variant_type == rttr::type::get<signed char>()) {
    try {
      v = node.as<signed char>();
    } catch (...) {}
    return;
  }

  if (variant_type == rttr::type::get<unsigned char>()) {
    try {
      v = node.as<unsigned char>();
    } catch (...) {}
    return;
  }

  if (variant_type == rttr::type::get<short>()) {
    try {
      v = node.as<short>();
    } catch (...) {}
    return;
  }

  if (variant_type == rttr::type::get<unsigned short>()) {
    try {
      v = node.as<unsigned short>();
    } catch (...) {}
    return;
  }

  if (variant_type == rttr::type::get<int>()) {
    try {
      v = node.as<int>();
    } catch (...) {}
    return;
  }

  if (variant_type == rttr::type::get<unsigned>()) {
    try {
      v = node.as<unsigned>();
    } catch (...) {}
    return;
  }

  if (variant_type == rttr::type::get<long>()) {
    try {
      v = node.as<long>();
    } catch (...) {}
    return;
  }

  if (variant_type == rttr::type::get<unsigned long>()) {
    try {
      v = node.as<unsigned long>();
    } catch (...) {}
    return;
  }

  if (variant_type == rttr::type::get<long long>()) {
    try {
      v = node.as<long long>();
    } catch (...) {}
    return;
  }

  if (variant_type == rttr::type::get<unsigned long long>()) {
    try {
      v = node.as<unsigned long long>();
    } catch (...) {}
    return;
  }

  if (variant_type == rttr::type::get<float>()) {
    try {
      v = node.as<float>();
    } catch (...) {}
    return;
  }

  if (variant_type == rttr::type::get<double>()) {
    try {
      v = node.as<double>();
    } catch (...) {}
    return;
  }

  if (variant_type == rttr::type::get<long double>()) {
    try {
      v = node.as<long double>();
    } catch (...) {}
    return;
  }

  if (variant_type == rttr::type::get<std::string>()) {
    try {
      v = node.as<std::string>();
    } catch (...) {}
    return;
  }

  if (variant_type.is_enumeration()) {
    auto const enumeration{variant_type.get_enumeration()};
    assert(enumeration.is_valid());
    auto copy{v};
    assert(copy.is_valid());
    auto success{copy.convert(enumeration.get_underlying_type())};
    assert(success);
    ReflectionDeserializeFromYaml(node, copy, ctx, extension_func);
    assert(copy.is_valid());
    success = copy.convert(enumeration.get_type());
    assert(success);
    v = copy;
    return;
  }

  // T*, ObserverPtr<T>, etc.
  if (variant_type.is_pointer() || (variant_type.is_wrapper() && variant_type.get_wrapped_type().is_pointer())) {
    // std::remove_pointer<T>
    auto const ptr_type{variant_type.is_pointer() ? variant_type : variant_type.get_wrapped_type()};

    // std::remove_cv<T>
    auto const raw_type{ptr_type.get_raw_type()};

    // std::derived_from<T, Resource>
    if (raw_type.is_derived_from(rttr::type::get<Resource>())) {
      auto const res_id{DeserializeResourceId(node, ctx)};

      if (res_id->IsValid()) {
        auto const res = App::Instance().GetResourceManager().GetOrLoad(*res_id);

        if (res) {
          rttr::variant res_var{res};

          // dynamic_cast<T*>(res)
          if (res_var.convert(ptr_type)) {
            // if the original type is a wrapper, we wrap the pointer here
            if (res_var.convert(variant_type)) {
              v = res_var;
            }
          }
        }
      }
    }
  }

  if (v.is_sequential_container()) {
    auto container{v.create_sequential_view()};
    assert(container.is_valid());

    if (container.is_dynamic()) {
      [[maybe_unused]] auto const success{container.set_size(node.size())};
      assert(success);
    }

    for (std::size_t i{0}; i < node.size() && container.get_size(); i++) {
      auto value{container.get_value(i).extract_wrapped_value()};
      assert(value.is_valid());
      ReflectionDeserializeFromYaml(node[i], value, ctx, extension_func);
      assert(value.is_valid());
      [[maybe_unused]] auto const success{container.set_value(i, value)};
      assert(success);
    }

    return;
  }

  if (v.is_associative_container()) {
    return;
  }

  if (variant_type.is_class()) {
    for (auto const& prop : variant_type.get_properties()) {
      auto value{prop.get_value(v)};
      assert(value.is_valid());
      ReflectionDeserializeFromYaml(node[prop.get_name().to_string()], value, ctx, extension_func);
      assert(value.is_valid());
      [[maybe_unused]] auto const success{prop.set_value(v, value)};
      assert(success);
    }

    return;
  }

  if (variant_type.is_wrapper() && variant_type.get_wrapped_type().is_class()) {
    for (auto const& prop : variant_type.get_wrapped_type().get_properties()) {
      auto value{prop.get_value(v)};
      assert(value.is_valid());
      ReflectionDeserializeFromYaml(node[prop.get_name().to_string()], value, ctx, extension_func);
      assert(value.is_valid());
      [[maybe_unused]] auto const success{prop.set_value(v, value)};
      assert(success);
    }

    return;
  }

  if (extension_func) {
    extension_func(node, v, ctx);
  }
}


auto ReflectionDeserializeFromYaml(
  YAML::Node const& node,
  rttr::variant&& v,
  YamlDeserializeContext const& ctx,
  std::function<void(YAML::Node const&, rttr::variant&, YamlDeserializeContext const&)> const& extension_func
) noexcept -> void {
  ReflectionDeserializeFromYaml(node, v, ctx, extension_func);
}


auto SerializeToBinary(std::string_view const sv, std::vector<std::byte>& bytes) noexcept -> void {
  SerializeToBinary(std::size(sv), bytes);

  if (!sv.empty()) {
    auto const sizeBeforeChars{std::size(bytes)};
    bytes.resize(sizeBeforeChars + std::size(sv));
    std::memcpy(&bytes[sizeBeforeChars], sv.data(), std::size(sv));
  }
}


auto DeserializeFromBinary(std::span<std::byte const> const bytes, std::string& str) noexcept -> bool {
  if (std::size(bytes) < 8) {
    return false;
  }

  std::uint64_t len;
  if (!DeserializeFromBinary(bytes, len)) {
    return false;
  }

  if (std::size(bytes) - 8 < len) {
    return false;
  }

  if (len == 0) {
    str.clear();
    return true;
  }

  str.resize(len);

  std::memcpy(str.data(), bytes.subspan(8).data(), len);
  return true;
}
}
