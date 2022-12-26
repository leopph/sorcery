#include "Model.hpp"

#include "ModelImport.hpp"
#include "Systems.hpp"

#include <assimp/scene.h>
#include <stdexcept>


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
		bufferData->metallic = 0;
		bufferData->roughness = 0.5f;
		bufferData->ao = 0;
		gRenderer.GetImmediateContext()->Unmap(mBuffer.Get(), 0);
	}


	auto Material::SetAlbedoColor(Color const& albedoColor) const noexcept -> void {
		D3D11_MAPPED_SUBRESOURCE mappedBuf;
		gRenderer.GetImmediateContext()->Map(mBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuf);
		static_cast<BufferData*>(mappedBuf.pData)->albedo = Vector3{ albedoColor.red / 255.f, albedoColor.green / 255.f, albedoColor.blue / 255.f };
		gRenderer.GetImmediateContext()->Unmap(mBuffer.Get(), 0);
	}


	auto Material::SetMetallic(f32 const metallic) const noexcept -> void {
		D3D11_MAPPED_SUBRESOURCE mappedBuf;
		gRenderer.GetImmediateContext()->Map(mBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuf);
		static_cast<BufferData*>(mappedBuf.pData)->metallic = metallic;
		gRenderer.GetImmediateContext()->Unmap(mBuffer.Get(), 0);
	}


	auto Material::SetRoughness(f32 const roughness) const noexcept -> void {
		D3D11_MAPPED_SUBRESOURCE mappedBuf;
		gRenderer.GetImmediateContext()->Map(mBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuf);
		static_cast<BufferData*>(mappedBuf.pData)->roughness = roughness;
		gRenderer.GetImmediateContext()->Unmap(mBuffer.Get(), 0);
	}


	auto Material::SetAo(f32 const ao) const noexcept -> void {
		D3D11_MAPPED_SUBRESOURCE mappedBuf;
		gRenderer.GetImmediateContext()->Map(mBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuf);
		static_cast<BufferData*>(mappedBuf.pData)->ao = ao;
		gRenderer.GetImmediateContext()->Unmap(mBuffer.Get(), 0);
	}


	auto Material::GetBuffer() const noexcept -> NonOwning<ID3D11Buffer*> {
		return mBuffer.Get();
	}


	auto Material::BindPs() const noexcept -> void {
		gRenderer.GetImmediateContext()->PSSetConstantBuffers(0, 1, mBuffer.GetAddressOf());
	}
}
