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

auto Material::GetMetallicMap() const noexcept -> Texture2D* {
	return mMetallicMap;
}

auto Material::SetMetallicMap(Texture2D* const tex) noexcept -> void {
	mMetallicMap = tex;
	mBufData.sampleMetallic = mMetallicMap != nullptr;
	UpdateGPUData();
}

auto Material::GetRoughnessMap() const noexcept -> Texture2D* {
	return mRoughnessMap;
}

auto Material::SetRoughnessMap(Texture2D* const tex) noexcept -> void {
	mRoughnessMap = tex;
	mBufData.sampleRoughness = mRoughnessMap != nullptr;
	UpdateGPUData();
}

auto Material::GetAoMap() const noexcept -> Texture2D* {
	return mAoMap;
}

auto Material::SetAoMap(Texture2D* const tex) noexcept -> void {
	mAoMap = tex;
	mBufData.sampleAo = mAoMap != nullptr;
	UpdateGPUData();
}


auto Material::GetBuffer() const noexcept -> NonOwning<ID3D11Buffer*> {
	return mBuffer.Get();
}

auto Material::GetSerializationType() const -> Type {
	return Type::Material;
}

auto Material::Serialize(std::vector<std::uint8_t>& out) const noexcept -> void {
	BinarySerializer<Vector3>::Serialize(mBufData.albedo, out, std::endian::little);
	BinarySerializer<f32>::Serialize(mBufData.metallic, out, std::endian::little);
	BinarySerializer<f32>::Serialize(mBufData.roughness, out, std::endian::little);
	BinarySerializer<f32>::Serialize(mBufData.ao, out, std::endian::little);

	BinarySerializer<u8>::Serialize(mBufData.sampleAlbedo != 0, out);
	BinarySerializer<u8>::Serialize(mBufData.sampleMetallic != 0, out);
	BinarySerializer<u8>::Serialize(mBufData.sampleRoughness != 0, out);
	BinarySerializer<u8>::Serialize(mBufData.sampleAo != 0, out);

	auto const trySerializeMap{
		[&out](Texture2D const* const map) {
			if (map) {
				auto const guidStr{ map->GetGuid().ToString() };
				BinarySerializer<u64>::Serialize(guidStr.size(), out, std::endian::little);
				std::ranges::transform(guidStr, std::back_inserter(out), [](char const c) {
					return static_cast<uint8_t>(c);
				});
			}
		}
	};

	trySerializeMap(mAlbedoMap);
	trySerializeMap(mMetallicMap);
	trySerializeMap(mRoughnessMap);
	trySerializeMap(mAoMap);
}

auto Material::Deserialize(std::span<std::uint8_t const> bytes) -> void {
	SetAlbedoVector(BinarySerializer<Vector3>::Deserialize(bytes.first<sizeof(Vector3)>(), std::endian::little));
	bytes = bytes.subspan(sizeof(Vector3));

	SetMetallic(BinarySerializer<f32>::Deserialize(bytes.first<sizeof(f32)>(), std::endian::little));
	bytes = bytes.subspan(sizeof(f32));

	SetRoughness(BinarySerializer<f32>::Deserialize(bytes.first<sizeof(f32)>(), std::endian::little));
	bytes = bytes.subspan(sizeof(f32));

	SetAo(BinarySerializer<f32>::Deserialize(bytes.first<sizeof(f32)>(), std::endian::little));
	bytes = bytes.subspan(sizeof(f32));

	auto const sampleAlbedo{ static_cast<bool>(BinarySerializer<u8>::Deserialize(bytes.first<sizeof(u8)>())) };
	bytes = bytes.subspan(sizeof(u8));

	auto const sampleMetallic{ static_cast<bool>(BinarySerializer<u8>::Deserialize(bytes.first<sizeof(u8)>())) };
	bytes = bytes.subspan(sizeof(u8));

	auto const sampleRoughness{ static_cast<bool>(BinarySerializer<u8>::Deserialize(bytes.first<sizeof(u8)>())) };
	bytes = bytes.subspan(sizeof(u8));

	auto const sampleAo{ static_cast<bool>(BinarySerializer<u8>::Deserialize(bytes.first<sizeof(u8)>())) };
	bytes = bytes.subspan(sizeof(u8));

	auto const tryParseAndSetMap{
		[&bytes](bool const cond, auto&& mapSetFn) {
			if (cond) {
				auto const guidStrLength{ BinarySerializer<u64>::Deserialize(bytes.first<sizeof(u64)>(), std::endian::little) };
				bytes = bytes.subspan(sizeof(u64));

				if (bytes.size() >= guidStrLength) {
					std::string guidStr;
					std::ranges::copy_n(std::begin(bytes), guidStrLength, std::back_inserter(guidStr));
					mapSetFn(dynamic_cast<Texture2D*>(FindObjectByGuid(Guid::Parse(guidStr))));
					bytes = bytes.subspan(guidStrLength);
				}
			}
			else {
				mapSetFn(nullptr);
			}
		}
	};

	tryParseAndSetMap(sampleAlbedo, [this](Texture2D* const tex) {
		SetAlbedoMap(tex);
	});

	tryParseAndSetMap(sampleMetallic, [this](Texture2D* const tex) {
		SetMetallicMap(tex);
	});

	tryParseAndSetMap(sampleRoughness, [this](Texture2D* const tex) {
		SetRoughnessMap(tex);
	});

	tryParseAndSetMap(sampleAo, [this](Texture2D* const tex) {
		SetAoMap(tex);
	});
}
}
