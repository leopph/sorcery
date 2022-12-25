#include "LeopphModelSerialize.hpp"

#include <iterator>


namespace leopph {
	auto Serialize(std::string_view const str, std::vector<u8>& oBuf, std::endian const endianness) -> void {
		auto const sz = static_cast<u64>(str.size());
		Serialize(sz, oBuf, endianness);

		std::copy_n(reinterpret_cast<u8 const*>(str.data()), sz, std::back_inserter(oBuf));
	}


	auto Serialize(Image const& img, std::vector<u8>& oBuf, std::endian const endianness) -> void {
		auto const width = static_cast<i32>(img.get_width());
		Serialize(width, oBuf, endianness);

		auto const height = static_cast<i32>(img.get_height());
		Serialize(height, oBuf, endianness);

		auto const chans = static_cast<u8>(img.get_num_channels());
		Serialize(chans, oBuf);

		std::copy_n(img.get_data().data(), width * height * chans, std::back_inserter(oBuf));
	}


	auto Serialize(Color const& color, std::vector<u8>& oBuf) -> void {
		Serialize(color.red, oBuf);
		Serialize(color.green, oBuf);
		Serialize(color.blue, oBuf);
		Serialize(color.alpha, oBuf);
	}


	auto Serialize(MaterialData const& mat, std::vector<u8>& oBuf, std::endian const endianness) -> void {
		Serialize(mat.albedo, oBuf);
		Serialize(mat.metallic, oBuf, endianness);
		Serialize(mat.roughness, oBuf, endianness);
		Serialize(mat.ao, oBuf, endianness);

		u8 flags{ 0 };
		std::vector<u64> textures;

		if (mat.albedoMapIndex.has_value()) {
			flags |= 0x01;
			textures.push_back(mat.albedoMapIndex.value());
		}

		if (mat.metallicMapIndex.has_value()) {
			flags |= 0x02;
			textures.push_back(mat.metallicMapIndex.value());
		}

		if (mat.roughnessMapIndex.has_value()) {
			flags |= 0x04;
			textures.push_back(mat.roughnessMapIndex.value());
		}

		if (mat.aoMapIndex.has_value()) {
			flags |= 0x08;
			textures.push_back(mat.aoMapIndex.value());
		}

		Serialize(flags, oBuf);
	}


	auto Serialize(Vertex const& vert, std::vector<u8>& oBuf, std::endian const endianness) -> void {
		Serialize(vert.position, oBuf, endianness);
		Serialize(vert.normal, oBuf, endianness);
		Serialize(vert.uv, oBuf, endianness);
	}


	auto Serialize(MeshData const& mesh, std::vector<u8>& oBuf, std::endian const endianness) -> void {
		Serialize(static_cast<u64>(mesh.vertices.size()), oBuf, endianness);
		for (auto const& vert : mesh.vertices) {
			Serialize(vert, oBuf, endianness);
		}

		Serialize(static_cast<u64>(mesh.indices.size()), oBuf, endianness);
		for (auto const ind : mesh.indices) {
			Serialize(ind, oBuf, endianness);
		}

		Serialize(mesh.boundingBox, oBuf, endianness);
	}


	auto Serialize(ModelData const& model, std::vector<u8>& oBuf, std::endian const endianness) -> void {
		Serialize(static_cast<u64>(model.meshes.size()), oBuf, endianness);
		for (auto const& mesh : model.meshes) {
			Serialize(mesh, oBuf, endianness);
		}

		Serialize(static_cast<u64>(model.materials.size()), oBuf, endianness);
		for (auto const& material : model.materials) {
			Serialize(material, oBuf, endianness);
		}

		Serialize(static_cast<u64>(model.textures.size()), oBuf, endianness);
		for (auto const& texture : model.textures) {
			Serialize(texture, oBuf, endianness);
		}
	}


	auto Serialize(AABB const& aabb, std::vector<u8>& oBuf, std::endian const endianness) -> void {
		Serialize(aabb.min, oBuf, endianness);
		Serialize(aabb.max, oBuf, endianness);
	}
}
