#pragma once

#include "YamlInclude.hpp"
#include "Math.hpp"
#include "Color.hpp"
#include "Image.hpp"
#include "Util.hpp"

#include <array>
#include <bit>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>


namespace YAML {
template<typename T, std::size_t N>
struct convert<leopph::Vector<T, N>> {
	static auto encode(leopph::Vector<T, N> const& v) -> Node {
		Node node;
		node.SetStyle(YAML::EmitterStyle::Flow);
		for (std::size_t i = 0; i < N; i++) {
			node.push_back(v[i]);
		}
		return node;
	}

	static auto decode(Node const& node, leopph::Vector<T, N>& v) -> bool {
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
struct convert<leopph::Quaternion> {
	static auto encode(leopph::Quaternion const& q) -> Node {
		Node node;
		node.SetStyle(YAML::EmitterStyle::Flow);
		node.push_back(q.w);
		node.push_back(q.x);
		node.push_back(q.y);
		node.push_back(q.z);
		return node;
	}

	static auto decode(Node const& node, leopph::Quaternion& q) -> bool {
		if (!node.IsSequence() || node.size() != 4) {
			return false;
		}
		q.w = node[0].as<leopph::f32>();
		q.x = node[1].as<leopph::f32>();
		q.y = node[2].as<leopph::f32>();
		q.z = node[3].as<leopph::f32>();
		return true;
	}
};
}


namespace leopph {
template<typename T>
struct BinarySerializer;

template<Scalar T> requires(sizeof(T) == 1)
struct BinarySerializer<T> {
	auto constexpr static SerializedSize = 1;

	static auto Serialize(T const scalar, std::vector<std::uint8_t>& out) -> void {
		out.emplace_back(*reinterpret_cast<std::uint8_t const*>(&scalar));
	}

	static auto Deserialize(std::span<u8 const, 1> const bytes) -> T {
		return *reinterpret_cast<T const*>(bytes.data());
	}
};

template<Scalar T> requires(sizeof(T) > 1)
struct BinarySerializer<T> {
	auto constexpr static SerializedSize = sizeof(T);

	static auto Serialize(T const scalar, std::vector<std::uint8_t>& out, std::endian const endianness) -> void {
		auto const* const begin = reinterpret_cast<u8 const*>(&scalar);
		auto const sz = sizeof(T);
		auto const inserter = std::back_inserter(out);

		if (endianness == std::endian::native) {
			std::copy_n(begin, sz, inserter);
			return;
		}

		std::copy_n(std::reverse_iterator{ begin + sz }, sz, inserter);
	}

	static auto Deserialize(std::span<u8 const, sizeof(T)> const bytes, std::endian const endianness) -> T {
		if (endianness == std::endian::native) {
			return *reinterpret_cast<T const*>(bytes.data());
		}

		auto static constexpr typeSz = sizeof(T);
		std::array<u8, typeSz> tmpBytes{};
		std::copy_n(std::reverse_iterator{ bytes.data() + typeSz }, typeSz, tmpBytes.data());
		return *reinterpret_cast<T const*>(tmpBytes.data());
	}
};

template<>
struct BinarySerializer<Image> {
	static auto Serialize(Image const& img, std::vector<std::uint8_t>& out, std::endian const endianness) -> void {
		BinarySerializer<u32>::Serialize(img.get_width(), out, endianness);
		BinarySerializer<u32>::Serialize(img.get_height(), out, endianness);
		BinarySerializer<u8>::Serialize(img.get_num_channels(), out);
		std::copy_n(img.get_data().data(), img.get_width() * img.get_height() * img.get_num_channels(), std::back_inserter(out));
	}

	static auto Deserialize(std::span<u8 const> const bytes, std::endian const endianness) -> Image {
		if (bytes.size() < 9) {
			throw std::runtime_error{ "Failed to serialize Image, file does not contain enough bytes to read image dimensions." };
		}

		auto const width{ BinarySerializer<u32>::Deserialize(bytes.subspan<0, 4>(), endianness) };
		auto const height{ BinarySerializer<u32>::Deserialize(bytes.subspan<4, 4>(), endianness) };
		auto const chans{ BinarySerializer<u8>::Deserialize(bytes.subspan<8, 1>()) };

		auto const imgSize{ static_cast<u64>(width) * height * chans };

		if (bytes.size() < 9 + imgSize) {
			throw std::runtime_error{ "Failed to serialize Image, file does not contain enough bytes to read image pixel data." };
		}

		auto imgData{ std::make_unique<unsigned char[]>(imgSize) };
		std::copy_n(std::begin(bytes), imgSize, imgData.get());
		return Image{ width, height, chans, std::move(imgData) };
	}
};

template<>
struct BinarySerializer<Color> {
	auto constexpr static SerializedSize = sizeof(Color);

	static auto Serialize(Color const& color, std::vector<std::uint8_t>& out) -> void {
		BinarySerializer<u8>::Serialize(color.red, out);
		BinarySerializer<u8>::Serialize(color.green, out);
		BinarySerializer<u8>::Serialize(color.blue, out);
		BinarySerializer<u8>::Serialize(color.alpha, out);
	}

	static auto Deserialize(std::span<u8 const, sizeof(Color)> const bytes) -> Color {
		return Color{
			BinarySerializer<u8>::Deserialize(bytes.subspan<0, 1>()),
			BinarySerializer<u8>::Deserialize(bytes.subspan<1, 1>()),
			BinarySerializer<u8>::Deserialize(bytes.subspan<2, 1>()),
			BinarySerializer<u8>::Deserialize(bytes.subspan<3, 1>())
		};
	}
};


template<typename T, u64 N>
struct BinarySerializer<Vector<T, N>> {
	auto constexpr static SerializedSize = sizeof(Vector<T, N>);

	static auto Serialize(Vector<T, N> const& vector, std::vector<std::uint8_t>& out, std::endian const endianness) -> void {
		for (u64 i{ 0 }; i < N; i++) {
			if constexpr (std::is_invocable_v<decltype(&BinarySerializer<T>::Serialize), T, std::vector<std::uint8_t>&, std::endian>) {
				BinarySerializer<T>::Serialize(vector[i], out, endianness);
			}
			else {
				BinarySerializer<T>::Serialize(vector[i], out);
			}
		}
	}

	static auto Deserialize(std::span<u8 const, sizeof(Vector<T, N>)> const bytes, std::endian const endianness) -> Vector<T, N> {
		Vector<T, N> ret{};
		for (std::size_t i{ 0 }; i < N; i++) {
			if constexpr (std::is_invocable_v<decltype(&BinarySerializer<T>::Deserialize), std::span<u8 const, sizeof(T)>, std::endian>) {
				ret[i] = BinarySerializer<T>::Deserialize(bytes.subspan(i * sizeof(T)).template first<sizeof(T)>(), endianness);
			}
			else {
				ret[i] = BinarySerializer<T>::Deserialize(bytes.subspan(i * sizeof(T)).template first<sizeof(T)>());
			}
		}
		return ret;
	}
};
}
