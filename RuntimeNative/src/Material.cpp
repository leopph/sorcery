#include "Material.hpp"

#include "Renderer.hpp"
#include "Serialization.hpp"

#include <assimp/scene.h>

#include <cstdint>
#include <stdexcept>


namespace leopph {
Object::Type const Material::SerializationType{ Type::Material };


auto Material::UpdateGPUData() const noexcept -> void {
	D3D11_MAPPED_SUBRESOURCE mappedCB;
	renderer::GetImmediateContext()->Map(mCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCB);
	*static_cast<ShaderMaterial*>(mappedCB.pData) = mShaderMtl;
	renderer::GetImmediateContext()->Unmap(mCB.Get(), 0);
}


auto Material::CreateCB() -> void {
	D3D11_BUFFER_DESC constexpr cbDesc{
		.ByteWidth = clamp_cast<UINT>(RoundToNextMultiple(sizeof(ShaderMaterial), 16)),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	D3D11_SUBRESOURCE_DATA const initialData{
		.pSysMem = &mShaderMtl,
		.SysMemPitch = 0,
		.SysMemSlicePitch = 0
	};

	if (FAILED(renderer::GetDevice()->CreateBuffer(&cbDesc, &initialData, mCB.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create material CB." };
	}
}


Material::Material() {
	CreateCB();
}


Material::Material(Vector3 const& albedoVector, float const metallic, float const roughness, float const ao, Texture2D* const albedoMap, Texture2D* const metallicMap, Texture2D* const roughnessMap, Texture2D* const aoMap, Texture2D* const normalMap) :
	mShaderMtl{
		.albedo = albedoVector,
		.metallic = metallic,
		.roughness = roughness,
		.ao = ao,
		.sampleAlbedo = albedoMap != nullptr,
		.sampleMetallic = metallicMap != nullptr,
		.sampleRoughness = roughnessMap != nullptr,
		.sampleAo = aoMap != nullptr,
		.sampleNormal = normalMap != nullptr
	},
	mAlbedoMap{ albedoMap },
	mMetallicMap{ metallicMap },
	mRoughnessMap{ roughnessMap },
	mAoMap{ aoMap },
	mNormalMap{ normalMap } {
	CreateCB();
}


auto Material::GetAlbedoVector() const noexcept -> Vector3 {
	return mShaderMtl.albedo;
}


auto Material::SetAlbedoVector(Vector3 const& albedoVector) noexcept -> void {
	mShaderMtl.albedo = albedoVector;
	UpdateGPUData();
}


auto Material::GetAlbedoColor() const noexcept -> Color {
	auto const mulColorVec{ GetAlbedoVector() * 255 };
	return Color{ static_cast<u8>(mulColorVec[0]), static_cast<u8>(mulColorVec[1]), static_cast<u8>(mulColorVec[2]), static_cast<u8>(mulColorVec[3]) };
}


auto Material::SetAlbedoColor(Color const& albedoColor) noexcept -> void {
	SetAlbedoVector(Vector3{ static_cast<float>(albedoColor.red) / 255.f, static_cast<float>(albedoColor.green) / 255.f, static_cast<float>(albedoColor.blue) / 255.f });
}


auto Material::GetMetallic() const noexcept -> f32 {
	return mShaderMtl.metallic;
}


auto Material::SetMetallic(f32 const metallic) noexcept -> void {
	mShaderMtl.metallic = metallic;
	UpdateGPUData();
}


auto Material::GetRoughness() const noexcept -> f32 {
	return mShaderMtl.roughness;
}


auto Material::SetRoughness(f32 const roughness) noexcept -> void {
	mShaderMtl.roughness = roughness;
	UpdateGPUData();
}


auto Material::GetAo() const noexcept -> f32 {
	return mShaderMtl.ao;
}


auto Material::SetAo(f32 const ao) noexcept -> void {
	mShaderMtl.ao = ao;
	UpdateGPUData();
}


auto Material::GetAlbedoMap() const noexcept -> Texture2D* {
	return mAlbedoMap;
}


auto Material::SetAlbedoMap(Texture2D* const tex) noexcept -> void {
	mAlbedoMap = tex;
	mShaderMtl.sampleAlbedo = mAlbedoMap != nullptr;
	UpdateGPUData();
}


auto Material::GetMetallicMap() const noexcept -> Texture2D* {
	return mMetallicMap;
}


auto Material::SetMetallicMap(Texture2D* const tex) noexcept -> void {
	mMetallicMap = tex;
	mShaderMtl.sampleMetallic = mMetallicMap != nullptr;
	UpdateGPUData();
}


auto Material::GetRoughnessMap() const noexcept -> Texture2D* {
	return mRoughnessMap;
}


auto Material::SetRoughnessMap(Texture2D* const tex) noexcept -> void {
	mRoughnessMap = tex;
	mShaderMtl.sampleRoughness = mRoughnessMap != nullptr;
	UpdateGPUData();
}


auto Material::GetAoMap() const noexcept -> Texture2D* {
	return mAoMap;
}


auto Material::SetAoMap(Texture2D* const tex) noexcept -> void {
	mAoMap = tex;
	mShaderMtl.sampleAo = mAoMap != nullptr;
	UpdateGPUData();
}


auto Material::GetNormalMap() const noexcept -> Texture2D* {
	return mNormalMap;
}


auto Material::SetNormalMap(Texture2D* const tex) noexcept -> void {
	mNormalMap = tex;
	mShaderMtl.sampleNormal = mNormalMap != nullptr;
	UpdateGPUData();
}


auto Material::GetBuffer() const noexcept -> NonOwning<ID3D11Buffer*> {
	return mCB.Get();
}


auto Material::GetSerializationType() const -> Type {
	return SerializationType;
}


auto Material::Serialize(std::vector<std::uint8_t>& out) const noexcept -> void {
	auto constexpr endianness{ std::endian::little };
	std::int8_t constexpr static MATERIAL_VERSION{ 1 };

	BinarySerializer<std::int8_t>::Serialize(MATERIAL_VERSION, endianness, out);
	BinarySerializer<Vector3>::Serialize(mShaderMtl.albedo, endianness, out);
	BinarySerializer<f32>::Serialize(mShaderMtl.metallic, endianness, out);
	BinarySerializer<f32>::Serialize(mShaderMtl.roughness, endianness, out);
	BinarySerializer<f32>::Serialize(mShaderMtl.ao, endianness, out);

	BinarySerializer<u8>::Serialize(mShaderMtl.sampleAlbedo != 0, endianness, out);
	BinarySerializer<u8>::Serialize(mShaderMtl.sampleMetallic != 0, endianness, out);
	BinarySerializer<u8>::Serialize(mShaderMtl.sampleRoughness != 0, endianness, out);
	BinarySerializer<u8>::Serialize(mShaderMtl.sampleAo != 0, endianness, out);
	BinarySerializer<u8>::Serialize(mShaderMtl.sampleNormal != 0, endianness, out);

	auto const trySerializeMap{
		[&out](Texture2D const* const map) {
			if (map) {
				BinarySerializer<std::string>::Serialize(map->GetGuid().ToString(), endianness, out);
			}
		}
	};

	trySerializeMap(mAlbedoMap);
	trySerializeMap(mMetallicMap);
	trySerializeMap(mRoughnessMap);
	trySerializeMap(mAoMap);
	trySerializeMap(mNormalMap);
}
}
