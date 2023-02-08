#include "Texture2D.hpp"

#include "Util.hpp"
#include "Systems.hpp"
#include "Renderer.hpp"

#pragma warning(push)
#pragma warning(disable: 26451 26819 6262)
//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#pragma warning(pop)

#include <cstring>


namespace leopph {
void Texture2D::UploadToGPU() {
	D3D11_TEXTURE2D_DESC const texDesc{
		.Width = clamp_cast<UINT>(mImgData.width),
		.Height = clamp_cast<UINT>(mImgData.height),
		.MipLevels = 1,
		.ArraySize = 1,
		.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
		.SampleDesc = { .Count = 1, .Quality = 0 },
		.Usage = D3D11_USAGE_IMMUTABLE,
		.BindFlags = D3D11_BIND_SHADER_RESOURCE,
		.CPUAccessFlags = 0,
		.MiscFlags = 0
	};
	D3D11_SUBRESOURCE_DATA const texData{
		.pSysMem = mImgData.bytes.get()
	};
	gRenderer.GetDevice()->CreateTexture2D(&texDesc, &texData, mTex.ReleaseAndGetAddressOf());
}

auto Texture2D::GetSerializationType() const -> Type {
	return SerializationType;
}

void Texture2D::SerializeBinary(std::vector<u8>& out) const {}

Object::BinaryDeserializationResult Texture2D::DeserializeBinary(std::span<u8 const> const bytes) {
	mTmpImgData.bytes.reset(stbi_load_from_memory(bytes.data(), clamp_cast<int>(bytes.size()), &mTmpImgData.width, &mTmpImgData.height, &mTmpImgData.channelCount, 0));
	if (mTmpImgData.channelCount != 4) {
		auto paddedData{ std::make_unique_for_overwrite<std::uint8_t[]>(static_cast<std::size_t>(mTmpImgData.width * mTmpImgData.height * 4)) };
		auto const imgSize{ mTmpImgData.width * mTmpImgData.height };

		for (int i = 0; i < imgSize; i++) {
			std::memcpy(paddedData.get() + i * 4, mTmpImgData.bytes.get() + i * mTmpImgData.channelCount, mTmpImgData.channelCount);

			for (int j = mTmpImgData.channelCount; j < 3; j++) {
				paddedData[i * 4 + j] = 0;
			}

			paddedData[i * 4 + 3] = 255;
		}

		mTmpImgData.bytes = std::move(paddedData);
	}

	return { 0 };
}
}
