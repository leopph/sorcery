#include "Model.hpp"
#include "ModelImport.hpp"

#include "Systems.hpp"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include <algorithm>
#include <cstring>
#include <format>
#include <utility>
#include <iostream>


namespace leopph {
	Material::Material() {
		D3D11_BUFFER_DESC constexpr bufferDesc{
			.ByteWidth = sizeof(BufferData) + 16 - sizeof(BufferData) % 16,
			.Usage = D3D11_USAGE_DYNAMIC,
			.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
			.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		if (FAILED(gRenderer.GetDevice()->CreateBuffer(&bufferDesc, nullptr, mBuffer.GetAddressOf()))) {
			throw std::runtime_error{ "Failed to create buffer for material." };
		}

		D3D11_MAPPED_SUBRESOURCE mappedBuf;
		gRenderer.GetImmediateContext()->Map(mBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuf);
		auto const bufferData{ static_cast<BufferData*>(mappedBuf.pData) };
		bufferData->albedo = Vector3{ 1, 1, 1 };
		bufferData->albedoMapIndex = -1;
		bufferData->metallic = 0;
		bufferData->metallicMapIndex = -1;
		bufferData->roughness = 0.5f;
		bufferData->roughnessMapIndex = -1;
		bufferData->ao = 0;
		bufferData->aoMapIndex = -1;
		gRenderer.GetImmediateContext()->Unmap(mBuffer.Get(), 0);
	}


	auto Material::SetAlbedoColor(Color const& albedoColor) const noexcept -> void {
		D3D11_MAPPED_SUBRESOURCE mappedBuf;
		gRenderer.GetImmediateContext()->Map(mBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuf);
		static_cast<BufferData*>(mappedBuf.pData)->albedo = Vector3{ albedoColor.red / 255.f, albedoColor.green / 255.f, albedoColor.blue / 255.f };
		gRenderer.GetImmediateContext()->Unmap(mBuffer.Get(), 0);
	}

	auto Material::SetAlbedoMap(ID3D11Texture2D* const albedoMap) noexcept -> void {
		mAlbedoMap = albedoMap;
	}


	auto Material::SetMetallic(f32 const metallic) const noexcept -> void {
		D3D11_MAPPED_SUBRESOURCE mappedBuf;
		gRenderer.GetImmediateContext()->Map(mBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuf);
		static_cast<BufferData*>(mappedBuf.pData)->metallic = metallic;
		gRenderer.GetImmediateContext()->Unmap(mBuffer.Get(), 0);
	}

	auto Material::SetMetallicMap(ID3D11Texture2D* const metallicMap) noexcept -> void {
		mMetallicMap = metallicMap;
	}


	auto Material::SetRoughness(f32 const roughness) const noexcept -> void {
		D3D11_MAPPED_SUBRESOURCE mappedBuf;
		gRenderer.GetImmediateContext()->Map(mBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuf);
		static_cast<BufferData*>(mappedBuf.pData)->roughness = roughness;
		gRenderer.GetImmediateContext()->Unmap(mBuffer.Get(), 0);
	}

	auto Material::SetRoughnessMap(ID3D11Texture2D* const roughnessMap) noexcept -> void {
		mRoughnessMap = roughnessMap;
	}


	auto Material::SetAo(f32 const ao) const noexcept -> void {
		D3D11_MAPPED_SUBRESOURCE mappedBuf;
		gRenderer.GetImmediateContext()->Map(mBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuf);
		static_cast<BufferData*>(mappedBuf.pData)->ao = ao;
		gRenderer.GetImmediateContext()->Unmap(mBuffer.Get(), 0);
	}

	auto Material::SetAoMap(ID3D11Texture2D* const aoMap) noexcept -> void {
		mAoMap = aoMap;
	}


	auto Material::GetBuffer() const noexcept -> NonOwning<ID3D11Buffer*> {
		return mBuffer.Get();
	}

	auto Material::GetAlbedoMap() const noexcept -> NonOwning<ID3D11Texture2D*> {
		return mAlbedoMap.Get();
	}

	auto Material::GetMetallicMap() const noexcept -> NonOwning<ID3D11Texture2D*> {
		return mMetallicMap.Get();
	}

	auto Material::GetRoughnessMap() const noexcept -> NonOwning<ID3D11Texture2D*> {
		return mRoughnessMap.Get();
	}

	auto Material::GetAoMap() const noexcept -> NonOwning<ID3D11Texture2D*> {
		return mAoMap.Get();
	}

	auto Material::BindPs() const noexcept -> void {
		gRenderer.GetImmediateContext()->PSSetConstantBuffers(0, 1, mBuffer.GetAddressOf());
	}
}
