#pragma once

#include "Object.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>
#include <wrl/client.h>

#include <cstdint>
#include <memory>

#include "Image.hpp"


namespace leopph {
class Texture2D final : public Object {
	Microsoft::WRL::ComPtr<ID3D11Texture2D> mTex;
	Image mImgData{};
	Image mTmpImgData{};

	void UploadToGPU();

public:
	Texture2D() = default;
	LEOPPHAPI explicit Texture2D(Image img);

	[[nodiscard]] LEOPPHAPI auto GetImageData() const noexcept -> Image const&;
	LEOPPHAPI auto SetImageData(Image img) noexcept -> void;

	LEOPPHAPI auto Update() noexcept -> void;

	LEOPPHAPI Type constexpr static SerializationType{ Type::Texture2D };
	LEOPPHAPI [[nodiscard]] auto GetSerializationType() const -> Type override;
};
}
