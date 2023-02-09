#pragma once

#include "Object.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>
#include <wrl/client.h>

#include <cstdint>
#include <memory>


namespace leopph {
class Texture2D final : public Object {
	struct ImageData {
		int width;
		int height;
		int channelCount;
		std::unique_ptr<std::uint8_t[]> bytes;
	};

	Microsoft::WRL::ComPtr<ID3D11Texture2D> mTex;
	ImageData mImgData{};
	ImageData mTmpImgData{};

	void UploadToGPU();

public:
	LEOPPHAPI Type constexpr static SerializationType{ Type::Texture2D };
	LEOPPHAPI [[nodiscard]] auto GetSerializationType() const -> Type override;

	LEOPPHAPI auto SerializeBinary(std::vector<u8>& out) const -> void override;
	LEOPPHAPI [[nodiscard]] auto DeserializeBinary(std::span<u8 const> bytes) -> BinaryDeserializationResult override;
};
}
