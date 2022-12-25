#pragma once

#pragma once

#include "Color.hpp"
#include "Image.hpp"
#include "ModelData.hpp"
#include "Util.hpp"

#include <array>
#include <bit>
#include <span>
#include <stdexcept>
#include <vector>


namespace leopph {
	template<typename T>
	struct BinarySerializer;

	template<Scalar T> requires(sizeof(T) == 1)
	struct BinarySerializer<T> {
		auto constexpr static SerializedSize = 1;

		static auto Serialize(T const scalar, std::vector<u8>& out) -> void {
			out.emplace_back(*reinterpret_cast<u8 const*>(&scalar));
		}

		static auto Deserialize(std::span<u8 const, 1> const bytes) -> T {
			return *reinterpret_cast<T const*>(bytes.data());
		}
	};

	template<Scalar T> requires(sizeof(T) > 1)
	struct BinarySerializer<T> {
		auto constexpr static SerializedSize = sizeof(T);

		static auto Serialize(T const scalar, std::vector<u8>& out, std::endian const endianness) -> void {
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
		static auto Serialize(Image const& img, std::vector<u8>& out, std::endian const endianness) -> void {
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
			auto const height{ BinarySerializer<u32>::Deserialize(bytes.subspan<4, 4>(), endianness)};
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

		static auto Serialize(Color const& color, std::vector<u8>& out) -> void {
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

		static auto Serialize(Vector<T, N> const& vector, std::vector<u8>& out, std::endian const endianness) -> void {
			for (u64 i{ 0 }; i < N; i++) {
				if constexpr (std::is_invocable_v<decltype(&BinarySerializer<T>::Serialize), T, std::vector<u8>&, std::endian>) {
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

	template<>
	struct BinarySerializer<Vertex> {
		auto constexpr static SerializedSize = sizeof(Vertex);

		static auto Serialize(Vertex const& vertex, std::vector<u8>& out, std::endian const endianness) -> void {
			BinarySerializer<Vector3>::Serialize(vertex.position, out, endianness);
			BinarySerializer<Vector3>::Serialize(vertex.normal, out, endianness);
			BinarySerializer<Vector2>::Serialize(vertex.uv, out, endianness);
		}

		static auto Deserialize(std::span<u8 const, sizeof(Vertex)> const bytes, std::endian const endianness) -> Vertex {
			return Vertex{
				.position = BinarySerializer<Vector3>::Deserialize(bytes.subspan<0, sizeof(Vector3)>(), endianness),
				.normal = BinarySerializer<Vector3>::Deserialize(bytes.subspan<sizeof(Vector3), sizeof(Vector3)>(), endianness),
				.uv = BinarySerializer<Vector2>::Deserialize(bytes.subspan<2 * sizeof(Vector3), sizeof(Vector2)>(), endianness)
			};
		}
	};


	template<>
	struct BinarySerializer<MaterialData> {
		auto constexpr static SerializedSize{ sizeof(Color) + 3 * sizeof(f32) + sizeof(u8) + 4 * sizeof(u64) };

		static auto Serialize(MaterialData const& matData, std::vector<u8>& out, std::endian const endianness) -> void {
			BinarySerializer<Color>::Serialize(matData.albedo, out);
			BinarySerializer<f32>::Serialize(matData.metallic, out, endianness);
			BinarySerializer<f32>::Serialize(matData.roughness, out, endianness);
			BinarySerializer<f32>::Serialize(matData.ao, out, endianness);

			u8 textureFlags{ 0 };
			std::array<u64, 4> textures{};

			if (matData.albedoMapIndex.has_value()) {
				textureFlags |= 0x01;
				textures[0] = matData.albedoMapIndex.value();
			}

			if (matData.metallicMapIndex.has_value()) {
				textureFlags |= 0x02;
				textures[1] = matData.metallicMapIndex.value();
			}

			if (matData.roughnessMapIndex.has_value()) {
				textureFlags |= 0x04;
				textures[2] = matData.roughnessMapIndex.value();
			}

			if (matData.aoMapIndex.has_value()) {
				textureFlags |= 0x08;
				textures[3] = matData.aoMapIndex.value();
			}

			BinarySerializer<u8>::Serialize(textureFlags, out);
			for (std::size_t i{ 0 }; i < textures.size(); i++) {
				BinarySerializer<u64>::Serialize(textures[i], out, endianness);
			}
		}

		static auto Deserialize(std::span<u8 const, SerializedSize> const bytes, std::endian const endianness) -> MaterialData {
			MaterialData ret;

			ret.albedo = BinarySerializer<Color>::Deserialize(bytes.subspan<0, sizeof(Color)>());
			ret.metallic = BinarySerializer<f32>::Deserialize(bytes.subspan<sizeof(Color), sizeof(f32)>(), endianness);
			ret.roughness = BinarySerializer<f32>::Deserialize(bytes.subspan<sizeof(Color) + sizeof(f32), sizeof(f32)>(), endianness);
			ret.ao = BinarySerializer<f32>::Deserialize(bytes.subspan<sizeof(Color) + 2 * sizeof(Color), sizeof(f32)>(), endianness);

			auto const textureFlags{ BinarySerializer<u8>::Deserialize(bytes.subspan<sizeof(Color) + 3 * sizeof(f32), sizeof(u8)>()) };
			std::array<u64, 4> textures{};

			for (std::size_t i{ 0 }; i < textures.size(); i++) {
				textures[i] = BinarySerializer<u64>::Deserialize(bytes.subspan(sizeof(Color) + 3 * sizeof(f32) + sizeof(u8) + i * sizeof(u64)).first<sizeof(u64)>(), endianness);
			}

			if (textureFlags & 0x1) {
				ret.albedoMapIndex = textures[0];
			}
			if (textureFlags & 0x2) {
				ret.metallicMapIndex = textures[1];
			}
			if (textureFlags & 0x4) {
				ret.roughnessMapIndex = textures[2];
			}
			if (textureFlags & 0x8) {
				ret.aoMapIndex = textures[3];
			}

			return ret;
		}
	};

	template<>
	struct BinarySerializer<MeshData> {
		static auto Serialize(MeshData const& meshData, std::vector<u8>& out, std::endian const endianness) -> void {
			BinarySerializer<u64>::Serialize(meshData.vertices.size(), out, endianness);
			for (auto const& vert : meshData.vertices) {
				BinarySerializer<Vertex>::Serialize(vert, out, endianness);
			}

			BinarySerializer<u64>::Serialize(meshData.indices.size(), out, endianness);
			for (auto const ind : meshData.indices) {
				BinarySerializer<u32>::Serialize(ind, out, endianness);
			}
		}

		static auto Deserialize(std::span<u8 const> const bytes, std::endian const endianness) -> MeshData {
			MeshData ret;

			if (bytes.size() < 8) {
				throw std::runtime_error{ "Failed to deserialize MeshData, file does not contain enough bytes to read vertex count." };
			}

			auto const numVerts{ BinarySerializer<u64>::Deserialize(bytes.subspan<0, 8>(), endianness) };

			if (bytes.size() < 8 + numVerts * sizeof(Vertex)) {
				throw std::runtime_error{ "Failed to deserialize MeshData, file does not contain enough bytes to read vertices." };
			}

			ret.vertices.reserve(numVerts);

			for (std::size_t i{ 0 }; i < numVerts; i++) {
				ret.vertices.emplace_back(BinarySerializer<Vertex>::Deserialize(bytes.subspan(8 + i * sizeof(Vertex)).first<sizeof(Vertex)>(), endianness));
			}

			if (bytes.size() < 8 + numVerts * sizeof(Vertex) + 8) {
				throw std::runtime_error{ "Failed to deserialize MeshData, file does not contain enough bytes to read index count." };
			}

			auto const numInds{ BinarySerializer<u64>::Deserialize(bytes.subspan(8 + numVerts * sizeof(Vertex)).first<8>(), endianness) };

			if (bytes.size() < 8 + numVerts * sizeof(Vertex) + 8 + numInds * sizeof(u32)) {
				throw std::runtime_error{ "Failed to deserialize MeshData, file does not contain enough bytes to read indices." };
			}

			ret.indices.reserve(numInds);

			for (std::size_t i{ 0 }; i < numInds; i++) {
				ret.indices.emplace_back(BinarySerializer<u32>::Deserialize(bytes.subspan(8 + i * sizeof(Vertex) + 8 + i * sizeof(u32)).first<sizeof(u32)>(), endianness));
			}

			// TODO calculate AABB?

			return ret;
		}
	};

	template<>
	struct BinarySerializer<ModelData> {
		static auto Serialize(ModelData const& modelData, std::vector<u8>& out, std::endian const endianness) -> void {
			BinarySerializer<u64>::Serialize(modelData.meshes.size(), out, endianness);
			for (auto const& mesh : modelData.meshes) {
				BinarySerializer<MeshData>::Serialize(mesh, out, endianness);
			}

			BinarySerializer<u64>::Serialize(modelData.materials.size(), out, endianness);
			for (auto const& material : modelData.materials) {
				BinarySerializer<MaterialData>::Serialize(material, out, endianness);
			}

			BinarySerializer<u64>::Serialize(modelData.textures.size(), out, endianness);
			for (auto const& texture : modelData.textures) {
				BinarySerializer<Image>::Serialize(texture, out, endianness);
			}
		}

		static auto Deserialize(std::span<u8 const> const bytes, std::endian const endianness) -> ModelData {
			ModelData ret;

			if (bytes.size() < 8) {
				throw std::runtime_error{ "Failed to deserialize ModelData, file does not contain enough bytes to read mesh count." };
			}

			auto const numMeshes{ BinarySerializer<u64>::Deserialize(bytes.subspan<0, 8>(), endianness) };

			// We cannot really check the size further from here, because MeshData is also opaque. Unsafe? Implement dynamic checks in all deserializers instead of using static span extents?

			ret.meshes.reserve(numMeshes);

			for (std::size_t i{ 0 }; i < numMeshes; i++) {
				// How do we iterate here? We can't increase subspan offset, because we don't know how many bytes have been used up by the previous iteration.
				// ret.meshes.emplace_back(BinarySerializer<MeshData>::Deserialize(bytes.subspan(), endianness));
			}

			return ret;
		}
	};
}
