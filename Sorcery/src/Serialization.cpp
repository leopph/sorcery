#include "Serialization.hpp"


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
}
