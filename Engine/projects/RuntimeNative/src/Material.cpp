#include "Material.hpp"

#include "ModelImport.hpp"
#include "Systems.hpp"
#include "BinarySerializer.hpp"

#include <imgui.h>

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

	auto Material::GetAlbedoColor() const noexcept -> Color {
		auto const mulColorVec{ mBufData.albedo * 255 };
		return Color{ static_cast<u8>(mulColorVec[0]), static_cast<u8>(mulColorVec[1]), static_cast<u8>(mulColorVec[2]), static_cast<u8>(mulColorVec[3]) };
	}

	auto Material::SetAlbedoColor(Color const& albedoColor) noexcept -> void {
		mBufData.albedo = Vector3{ albedoColor.red / 255.f, albedoColor.green / 255.f, albedoColor.blue / 255.f };
		UpdateGPUData();
	}

	auto Material::SetAlbedoColor(Vector3 const& albedoColor) noexcept -> void {
		SetAlbedoColor(Color{ static_cast<u8>(albedoColor[0] * 255), static_cast<u8>(albedoColor[1] * 255), static_cast<u8>(albedoColor[2] * 255), 255 });
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


	auto Material::GetBuffer() const noexcept -> NonOwning<ID3D11Buffer*> {
		return mBuffer.Get();
	}


	auto Material::BindPs() const noexcept -> void {
		gRenderer.GetImmediateContext()->PSSetConstantBuffers(0, 1, mBuffer.GetAddressOf());
	}

	auto Material::GetSerializationType() const -> Type {
		return Type::Material;
	}

	auto Material::SerializeBinary(std::vector<u8>& out) const -> void {
		BinarySerializer<Vector3>::Serialize(mBufData.albedo, out, std::endian::native);
		BinarySerializer<f32>::Serialize(mBufData.metallic, out, std::endian::native);
		BinarySerializer<f32>::Serialize(mBufData.roughness, out, std::endian::native);
		BinarySerializer<f32>::Serialize(mBufData.ao, out, std::endian::native);
	}

	auto Material::DeserializeBinary(std::span<u8 const> bytes) -> Object::BinaryDeserializationResult {
		auto constexpr serializedSize{ sizeof(Vector3) + 3 * sizeof(f32) };

		if (bytes.size() < serializedSize) {
			throw std::runtime_error{ "Failed to deserialize Material, because span does not contain enough bytes to read data." };
		}

		mBufData.albedo = BinarySerializer<Vector3>::Deserialize(bytes.first<sizeof(Vector3)>(), std::endian::native);
		mBufData.metallic = BinarySerializer<f32>::Deserialize(bytes.subspan<sizeof(Vector3), sizeof(f32)>(), std::endian::native);
		mBufData.roughness = BinarySerializer<f32>::Deserialize(bytes.subspan<sizeof(Vector3) + sizeof(f32), sizeof(f32)>(), std::endian::native);
		mBufData.ao = BinarySerializer<f32>::Deserialize(bytes.subspan<sizeof(Vector3) + 2 * sizeof(f32), sizeof(f32)>(), std::endian::native);

		for (std::size_t i = 0; i < 4; i++) {
			mBufData.albedo[i] = std::clamp(mBufData.albedo[i], 0.f, 1.f);
		}
		mBufData.metallic = std::clamp(mBufData.metallic, 0.f, 1.f);
		mBufData.roughness = std::clamp(mBufData.roughness, 0.f, 1.f);
		mBufData.ao = std::clamp(mBufData.ao, 0.f, 1.f);

		return { serializedSize };
	}

	auto Material::OnGui() -> void {
		Object::OnGui();

		if (ImGui::BeginTable(std::format("{}", GetGuid().ToString()).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::PushItemWidth(FLT_MIN);
			ImGui::TableSetColumnIndex(1);
			ImGui::PushItemWidth(-FLT_MIN);

			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Albedo Color");
			ImGui::TableNextColumn();

			if (Vector3 albedoColor{ mBufData.albedo }; ImGui::ColorEdit3("###matAlbedoColor", albedoColor.get_data())) {
				SetAlbedoColor(albedoColor);
			}

			ImGui::TableNextColumn();
			ImGui::Text("Metallic");
			ImGui::TableNextColumn();

			if (f32 metallic{ GetMetallic() }; ImGui::SliderFloat("###matMetallic", &metallic, 0.0f, 1.0f)) {
				SetMetallic(metallic);
			}

			ImGui::TableNextColumn();
			ImGui::Text("Roughness");
			ImGui::TableNextColumn();

			if (f32 roughness{ GetRoughness() }; ImGui::SliderFloat("###matRoughness", &roughness, 0.0f, 1.0f)) {
				SetRoughness(roughness);
			}

			ImGui::TableNextColumn();
			ImGui::Text("Ambient Occlusion");
			ImGui::TableNextColumn();

			if (f32 ao{ GetAo() }; ImGui::SliderFloat("###matAo", &ao, 0.0f, 1.0f)) {
				SetAo(ao);
			}

			ImGui::EndTable();
		}
	}
}
