#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <format>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "Color.hpp"
#include "Core.hpp"
#include "Image.hpp"
#include "Math.hpp"
#include "Resources/Mesh.hpp"

// yaml-cpp incorrectly uses dllexport specifiers so we silence their warnings
#pragma warning (push)
#pragma warning (disable: 4251 4275)
#include <yaml-cpp/yaml.h>
#pragma warning (pop)


namespace YAML {
template<typename T, std::size_t N>
struct convert<sorcery::Vector<T, N>> {
  static auto encode(sorcery::Vector<T, N> const& v) -> Node {
    Node node;
    node.SetStyle(EmitterStyle::Flow);
    for (std::size_t i = 0; i < N; i++) {
      node.push_back(v[i]);
    }
    return node;
  }


  static auto decode(Node const& node, sorcery::Vector<T, N>& v) -> bool {
    if (!node.IsSequence() || node.size() != N) {
      return false;
    }
    for (std::size_t i = 0; i < N; i++) {
      v[i] = node[i].as<T>();
    }
    return true;
  }
};


template<>
struct convert<sorcery::Quaternion> {
  static auto encode(sorcery::Quaternion const& q) -> Node {
    Node node;
    node.SetStyle(EmitterStyle::Flow);
    node.push_back(q.w);
    node.push_back(q.x);
    node.push_back(q.y);
    node.push_back(q.z);
    return node;
  }


  static auto decode(Node const& node, sorcery::Quaternion& q) -> bool {
    if (!node.IsSequence() || node.size() != 4) {
      return false;
    }
    q.w = node[0].as<sorcery::f32>();
    q.x = node[1].as<sorcery::f32>();
    q.y = node[2].as<sorcery::f32>();
    q.z = node[3].as<sorcery::f32>();
    return true;
  }
};
}


namespace sorcery {
template<typename T>
struct BinarySerializer;


template<typename T> requires (std::is_scalar_v<T>)
struct BinarySerializer<T> {
  static auto Serialize(T const scalar, [[maybe_unused]] std::endian const endianness, std::vector<std::uint8_t>& out) -> void {
    if constexpr (sizeof(T) == 1) {
      out.emplace_back(*reinterpret_cast<std::uint8_t const*>(&scalar));
    } else {
      auto const* const begin = reinterpret_cast<std::uint8_t const*>(&scalar);
      auto const sz = sizeof(T);
      auto const inserter = std::back_inserter(out);

      if (endianness == std::endian::native) {
        std::copy_n(begin, sz, inserter);
        return;
      }

      std::copy_n(std::reverse_iterator{ begin + sz }, sz, inserter);
    }
  }


  static auto Deserialize(std::span<std::uint8_t const> bytes, [[maybe_unused]] std::endian const endianness, T& out) -> std::span<std::uint8_t const> {
    if (bytes.size() < sizeof(T)) {
      throw std::runtime_error{ std::format("Cannot deserialize scalar of size {} because the source byte array is only of length {}.", sizeof(T), bytes.size()) };
    }

    if constexpr (sizeof(T) == 1) {
      out = *reinterpret_cast<T const*>(bytes.data());
    } else {
      if (endianness == std::endian::native) {
        out = *reinterpret_cast<T const*>(bytes.data());
      } else {
        auto static constexpr typeSz = sizeof(T);
        std::array<std::uint8_t, typeSz> tmpBytes{};
        std::copy_n(std::reverse_iterator{ bytes.data() + typeSz }, typeSz, tmpBytes.data());
        out = *reinterpret_cast<T const*>(tmpBytes.data());
      }
    }

    return bytes.subspan(sizeof(T));
  }
};


template<typename T>
struct BinarySerializer<std::vector<T>> {
  static auto Serialize(std::span<T const> const arr, std::endian const endianness, std::vector<std::uint8_t>& out) -> void {
    BinarySerializer<std::uint64_t>::Serialize(std::size(arr), endianness, out);

    for (T const& elem : arr) {
      BinarySerializer<T>::Serialize(elem, endianness, out);
    }
  }


  static auto Deserialize(std::span<std::uint8_t const> bytes, std::endian const endianness, std::vector<T>& out) -> std::span<std::uint8_t const> {
    std::uint64_t arrLength;
    bytes = BinarySerializer<std::uint64_t>::Deserialize(bytes, endianness, arrLength);

    out.resize(arrLength);

    for (std::uint64_t i = 0; i < arrLength; i++) {
      bytes = BinarySerializer<T>::Deserialize(bytes, endianness, out[i]);
    }

    return bytes;
  }
};


template<typename T, int N>
struct BinarySerializer<Vector<T, N>> {
  static auto Serialize(Vector<T, N> const& vector, std::endian const endianness, std::vector<std::uint8_t>& out) -> void {
    for (int i = 0; i < N; i++) {
      BinarySerializer<T>::Serialize(vector[i], endianness, out);
    }
  }


  static auto Deserialize(std::span<std::uint8_t const> bytes, std::endian const endianness, Vector<T, N>& out) -> std::span<std::uint8_t const> {
    for (int i = 0; i < N; i++) {
      bytes = BinarySerializer<T>::Deserialize(bytes, endianness, out[i]);
    }

    return bytes;
  }
};


template<>
struct BinarySerializer<std::string> {
  LEOPPHAPI static auto Serialize(std::string_view str, std::endian endianness, std::vector<std::uint8_t>& out) -> void;
  LEOPPHAPI static auto Deserialize(std::span<std::uint8_t const> bytes, std::endian endianness, std::string& out) -> std::span<std::uint8_t const>;
};


template<>
struct BinarySerializer<Image> {
  LEOPPHAPI static auto Serialize(Image const& img, std::endian endianness, std::vector<std::uint8_t>& out) -> void;
  LEOPPHAPI static auto Deserialize(std::span<std::uint8_t const> bytes, std::endian endianness, Image& out) -> std::span<std::uint8_t const>;
};


template<>
struct BinarySerializer<Color> {
  LEOPPHAPI static auto Serialize(Color const& color, std::endian endianness, std::vector<std::uint8_t>& out) -> void;
  LEOPPHAPI static auto Deserialize(std::span<std::uint8_t const> bytes, std::endian endianness, Color& out) -> std::span<std::uint8_t const>;
};


template<>
struct BinarySerializer<Mesh::SubMeshData> {
  LEOPPHAPI static auto Serialize(Mesh::SubMeshData const& submeshData, std::endian endianness, std::vector<std::uint8_t>& out) -> void;
  LEOPPHAPI static auto Deserialize(std::span<std::uint8_t const> bytes, std::endian endianness, Mesh::SubMeshData& out) -> std::span<std::uint8_t const>;
};


template<>
struct BinarySerializer<Mesh::Data> {
  LEOPPHAPI static auto Serialize(Mesh::Data const& meshData, std::endian endianness, std::vector<std::uint8_t>& out) -> void;
  LEOPPHAPI static auto Deserialize(std::span<std::uint8_t const> bytes, std::endian endianness, Mesh::Data& out) -> std::span<std::uint8_t const>;
};
}
