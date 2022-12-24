#include "Material.hpp"

#include "Systems.hpp"


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
	}


	auto Material::SetAlbedoColor(Vector3 const& albedoColor) const noexcept -> void {
		D3D11_MAPPED_SUBRESOURCE mappedBuf;
		gRenderer.GetImmediateContext()->Map(mBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuf);
		static_cast<BufferData*>(mappedBuf.pData)->albedo = albedoColor;
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
}
