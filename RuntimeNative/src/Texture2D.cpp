#include "Texture2D.hpp"

#include "Util.hpp"
#include "Systems.hpp"
#include "Renderer.hpp"


namespace leopph {
void Texture2D::UploadToGPU() {
	D3D11_TEXTURE2D_DESC const texDesc{
		.Width = clamp_cast<UINT>(mImgData.get_width()),
		.Height = clamp_cast<UINT>(mImgData.get_height()),
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
		.pSysMem = mImgData.get_data().data(),
		.SysMemPitch = mImgData.get_width() * mImgData.get_num_channels()
	};
	gRenderer.GetDevice()->CreateTexture2D(&texDesc, &texData, mTex.ReleaseAndGetAddressOf());
}

Texture2D::Texture2D(Image img) : mTmpImgData{ std::move(img) } {
	Update();
}

auto Texture2D::GetImageData() const noexcept -> Image const& {
	return mImgData;
}

auto Texture2D::SetImageData(Image img) noexcept -> void {
	mTmpImgData = std::move(img);
}

auto Texture2D::Update() noexcept -> void {
	mImgData = mTmpImgData;
	UploadToGPU();
}

auto Texture2D::GetSerializationType() const -> Type {
	return SerializationType;
}
}
