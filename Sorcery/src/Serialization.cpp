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
}


namespace sorcery {
auto BinarySerializer<std::string>::Serialize(std::string_view const str, std::endian const endianness, std::vector<std::uint8_t>& out) -> void {
  BinarySerializer<std::size_t>::Serialize(std::size(str), endianness, out);

  for (int i = 0; i < std::ssize(str); i++) {
    BinarySerializer<char>::Serialize(str[i], endianness, out);
  }
}


auto BinarySerializer<std::string>::Deserialize(std::span<std::uint8_t const> bytes, std::endian const endianness, std::string& out) -> std::span<std::uint8_t const> {
  std::uint64_t strLength;
  bytes = BinarySerializer<std::size_t>::Deserialize(bytes, endianness, strLength);

  if (std::size(bytes) < strLength * sizeof(char)) {
    throw std::runtime_error{ std::format("Cannot deserialize string of length {} because the source byte array is only of length {}.", strLength, bytes.size()) };
  }

  out.resize(strLength);

  for (std::uint64_t i = 0; i < strLength; i++) {
    bytes = BinarySerializer<char>::Deserialize(bytes, endianness, out[i]);
  }

  return bytes;
}


auto BinarySerializer<Image>::Serialize(Image const& img, std::endian const endianness, std::vector<std::uint8_t>& out) -> void {
  BinarySerializer<std::uint32_t>::Serialize(img.get_width(), endianness, out);
  BinarySerializer<std::uint32_t>::Serialize(img.get_height(), endianness, out);
  BinarySerializer<std::uint8_t>::Serialize(img.get_num_channels(), endianness, out);
  std::copy_n(img.get_data().data(), img.get_width() * img.get_height() * img.get_num_channels(), std::back_inserter(out));
}


auto BinarySerializer<Image>::Deserialize(std::span<std::uint8_t const> bytes, std::endian const endianness, Image& out) -> std::span<std::uint8_t const> {
  std::uint32_t width;
  bytes = BinarySerializer<uint32_t>::Deserialize(bytes, endianness, width);

  std::uint32_t height;
  bytes = BinarySerializer<uint32_t>::Deserialize(bytes, endianness, height);

  std::uint8_t chans;
  bytes = BinarySerializer<u8>::Deserialize(bytes, endianness, chans);

  auto const imgSize{ static_cast<u64>(width) * height * chans };

  if (bytes.size() < imgSize) {
    throw std::runtime_error{ std::format("Cannot deserialize Image of byte size {} because the source byte array is only of length {}.", imgSize, bytes.size()) };
  }

  auto imgData{ std::make_unique<unsigned char[]>(imgSize) };
  std::copy_n(std::begin(bytes), imgSize, imgData.get());

  out = Image{ width, height, chans, std::move(imgData) };

  return bytes.subspan(imgSize);
}


auto BinarySerializer<Color>::Serialize(Color const& color, std::endian const endianness, std::vector<std::uint8_t>& out) -> void {
  BinarySerializer<std::uint8_t>::Serialize(color.red, endianness, out);
  BinarySerializer<std::uint8_t>::Serialize(color.green, endianness, out);
  BinarySerializer<std::uint8_t>::Serialize(color.blue, endianness, out);
  BinarySerializer<std::uint8_t>::Serialize(color.alpha, endianness, out);
}


auto BinarySerializer<Color>::Deserialize(std::span<std::uint8_t const> bytes, std::endian const endianness, Color& out) -> std::span<std::uint8_t const> {
  bytes = BinarySerializer<std::uint8_t>::Deserialize(bytes, endianness, out.red);
  bytes = BinarySerializer<std::uint8_t>::Deserialize(bytes, endianness, out.green);
  bytes = BinarySerializer<std::uint8_t>::Deserialize(bytes, endianness, out.blue);
  bytes = BinarySerializer<std::uint8_t>::Deserialize(bytes, endianness, out.alpha);
  return bytes;
}


auto BinarySerializer<Mesh::SubMeshData>::Serialize(Mesh::SubMeshData const& submeshData, std::endian const endianness, std::vector<std::uint8_t>& out) -> void {
  BinarySerializer<int>::Serialize(submeshData.baseVertex, endianness, out);
  BinarySerializer<int>::Serialize(submeshData.firstIndex, endianness, out);
  BinarySerializer<int>::Serialize(submeshData.indexCount, endianness, out);
  BinarySerializer<std::string>::Serialize(submeshData.mtlSlotName, endianness, out);
}


auto BinarySerializer<Mesh::SubMeshData>::Deserialize(std::span<std::uint8_t const> bytes, std::endian const endianness, Mesh::SubMeshData& out) -> std::span<std::uint8_t const> {
  bytes = BinarySerializer<int>::Deserialize(bytes, endianness, out.baseVertex);
  bytes = BinarySerializer<int>::Deserialize(bytes, endianness, out.firstIndex);
  bytes = BinarySerializer<int>::Deserialize(bytes, endianness, out.indexCount);
  bytes = BinarySerializer<std::string>::Deserialize(bytes, endianness, out.mtlSlotName);
  return bytes;
}


auto BinarySerializer<Mesh::Data>::Serialize(Mesh::Data const& meshData, std::endian const endianness, std::vector<std::uint8_t>& out) -> void {
  BinarySerializer<std::vector<Vector3>>::Serialize(meshData.positions, endianness, out);
  BinarySerializer<std::vector<Vector3>>::Serialize(meshData.normals, endianness, out);
  BinarySerializer<std::vector<Vector2>>::Serialize(meshData.uvs, endianness, out);
  BinarySerializer<std::vector<Vector3>>::Serialize(meshData.tangents, endianness, out);
  BinarySerializer<std::vector<std::uint32_t>>::Serialize(meshData.indices, endianness, out);
  BinarySerializer<std::vector<Mesh::SubMeshData>>::Serialize(meshData.subMeshes, endianness, out);
}


auto BinarySerializer<Mesh::Data>::Deserialize(std::span<std::uint8_t const> bytes, std::endian const endianness, Mesh::Data& out) -> std::span<std::uint8_t const> {
  bytes = BinarySerializer<std::vector<Vector3>>::Deserialize(bytes, endianness, out.positions);
  bytes = BinarySerializer<std::vector<Vector3>>::Deserialize(bytes, endianness, out.normals);
  bytes = BinarySerializer<std::vector<Vector2>>::Deserialize(bytes, endianness, out.uvs);
  bytes = BinarySerializer<std::vector<Vector3>>::Deserialize(bytes, endianness, out.tangents);
  bytes = BinarySerializer<std::vector<std::uint32_t>>::Deserialize(bytes, endianness, out.indices);
  bytes = BinarySerializer<std::vector<Mesh::SubMeshData>>::Deserialize(bytes, endianness, out.subMeshes);
  return bytes;
}


auto ReflectionSerializeToYAML(rttr::variant const& obj, std::function<YAML::Node(rttr::variant const&)> const& extensionFunc) -> YAML::Node {
  rttr::type const objType{ obj.get_type() };

  if (!objType.is_valid()) {
    return {};
  }

  YAML::Node retNode;
  retNode["type"] = objType.get_name().to_string();

  for (auto const prop : objType.get_properties()) {
    retNode["properties"].push_back(detail::ReflectionSerializeToYAML(prop.get_value(obj), extensionFunc));
  }

  return retNode;
}


auto ReflectionDeserializeFromYAML(YAML::Node const& objNode, rttr::variant& obj, std::function<void(YAML::Node const&, rttr::variant&)> const& extensionFunc) -> void {
  auto const nodeStoredType{
    rttr::type::get(objNode["type"]
                      ? objNode["type"].as<std::string>()
                      : "")
  };

  rttr::type const objType{ obj.get_type() };

  if (!nodeStoredType.is_valid() || !objType.is_valid() || nodeStoredType != objType) {
    return;
  }

  for (auto const prop : objType.get_properties()) {
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
