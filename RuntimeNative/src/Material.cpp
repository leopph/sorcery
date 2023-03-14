#include "Material.hpp"

#include "Systems.hpp"
#include "Serialization.hpp"

#include <assimp/scene.h>

#include <stdexcept>


namespace leopph {
Object::Type const Material::SerializationType{ Type::Material };

auto Material::UpdateGPUData() const noexcept -> void {
	D3D11_MAPPED_SUBRESOURCE mappedBuf;
	gRenderer.GetImmediateContext()->Map(mBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuf);
	auto const mappedBufData{ static_cast<BufferData*>(mappedBuf.pData) };
	mappedBufData->albedo = mBufData.albedo;
	mappedBufData->metallic = mBufData.metallic;
	mappedBufData->roughness = mBufData.roughness;
	mappedBufData->ao = mBufData.ao;
	mappedBufData->sampleAlbedo = mBufData.sampleAlbedo;
	gRenderer.GetImmediateContext()->Unmap(mBuffer.Get(), 0);
}


Material::Material() {
	D3D11_BUFFER_DESC constexpr bufferDesc{
		.ByteWidth = clamp_cast<UINT>(RoundToNextMultiple(sizeof(BufferData), 16)),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	if (FAILED(gRenderer.GetDevice()->CreateBuffer(&bufferDesc, nullptr, mBuffer.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create buffer for material." };
	}

	UpdateGPUData();
}

auto Material::GetAlbedoVector() const noexcept -> Vector3 {
	return mBufData.albedo;
}

auto Material::SetAlbedoVector(Vector3 const& albedoVector) noexcept -> void {
	mBufData.albedo = albedoVector;
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
	return mBufData.metallic;
}

auto Material::SetMetallic(f32 const metallic) noexcept -> void {
	mBufData.metallic = metallic;
	UpdateGPUData();
}

auto Material::GetRoughness() const noexcept -> f32 {
	return mBufData.roughness;
}

auto Material::SetRoughness(f32 const roughness) noexcept -> void {
	mBufData.roughness = roughness;
	UpdateGPUData();
}

auto Material::GetAo() const noexcept -> f32 {
	return mBufData.ao;
}

auto Material::SetAo(f32 const ao) noexcept -> void {
	mBufData.ao = ao;
	UpdateGPUData();
}

auto Material::GetAlbedoMap() const noexcept -> Texture2D* {
	return mAlbedoMap;
}

auto Material::SetAlbedoMap(Texture2D* const tex) noexcept -> void {
	mAlbedoMap = tex;
	mBufData.sampleAlbedo = mAlbedoMap != nullptr;
	UpdateGPUData();
}


auto Material::GetBuffer() const noexcept -> NonOwning<ID3D11Buffer*> {
	return mBuffer.Get();
}

auto Material::GetSerializationType() const -> Type {
	return Type::Material;
}

auto Material::Serialize(std::vector<std::uint8_t>& out) const noexcept -> void {
	BinarySerializer<Vector3>::Serialize(mBufData.albedo, out, std::endian::native);
	BinarySerializer<f32>::Serialize(mBufData.metallic, out, std::endian::native);
	BinarySerializer<f32>::Serialize(mBufData.roughness, out, std::endian::native);
	BinarySerializer<f32>::Serialize(mBufData.ao, out, std::endian::native);

	BinarySerializer<u8>::Serialize(mBufData.sampleAlbedo != 0, out);

	if (mAlbedoMap) {
		auto const albedoMapGuidStr{ mAlbedoMap->GetGuid().ToString() };
		BinarySerializer<u64>::Serialize(albedoMapGuidStr.size(), out, std::endian::native);

		out.reserve(out.size() + albedoMapGuidStr.size());
		std::ranges::transform(albedoMapGuidStr, std::back_inserter(out), [](char const c) {
			return static_cast<uint8_t>(c);
		});
	}
}

auto Material::Deserialize(std::span<std::uint8_t const> const bytes) -> void {
	SetAlbedoVector(BinarySerializer<Vector3>::Deserialize(bytes.first<sizeof(Vector3)>(), std::endian::native));
	SetMetallic(BinarySerializer<f32>::Deserialize(bytes.subspan<sizeof(Vector3), sizeof(f32)>(), std::endian::native));
	SetRoughness(BinarySerializer<f32>::Deserialize(bytes.subspan<sizeof(Vector3) + sizeof(f32), sizeof(f32)>(), std::endian::native));
	SetAo(BinarySerializer<f32>::Deserialize(bytes.subspan<sizeof(Vector3) + 2 * sizeof(f32), sizeof(f32)>(), std::endian::native));

	if (static_cast<bool>(BinarySerializer<u8>::Deserialize(bytes.subspan<sizeof(Vector3) + 3 * sizeof(f32), sizeof(u8)>()))) {
		auto const albedoMapGuidStrLngth{ BinarySerializer<u64>::Deserialize(bytes.subspan<sizeof(Vector3) + 3 * sizeof(f32) + sizeof(u8), sizeof(u64)>(), std::endian::native) };

		if (auto const guidStrBytes{ bytes.subspan<sizeof(Vector3) + 3 * sizeof(f32) + sizeof(u8) + sizeof(u64)>() }; guidStrBytes.size() >= albedoMapGuidStrLngth) {
			std::string albedoMapGuidStr;
			albedoMapGuidStr.reserve(albedoMapGuidStrLngth);

			for (u64 i = 0; i < albedoMapGuidStrLngth; i++) {
				albedoMapGuidStr.push_back(static_cast<char>(guidStrBytes[i]));
			}

			SetAlbedoMap(dynamic_cast<Texture2D*>(FindObjectByGuid(Guid::Parse(albedoMapGuidStr))));
		}
	}
	else {
		SetAlbedoMap(nullptr);
	}
}
}
