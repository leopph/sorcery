#include "CubemapLoader.hpp"

#include "Image.hpp"
#include "Cubemap.hpp"

#include <stb_image.h>

#include <array>
#include <algorithm>

#include "Math.hpp"


namespace sorcery {
auto mage::CubemapLoader::GetSupportedExtensions() const -> std::span<std::string const> {
  static std::array<std::string, 9> const extensions{
    "png", "jpg", "jpeg", "tga", "bmp", "psd", "gif", "hdr", "pic"
  };

  return extensions;
}


auto mage::CubemapLoader::Load(std::filesystem::path const& src, [[maybe_unused]] std::filesystem::path const& cache) -> std::unique_ptr<Object> {
  int width;
  int height;
  int channelCount;

  if (std::unique_ptr<u8, decltype([](u8* const p) {
    stbi_image_free(p);
  })> const data{ stbi_load(src.string().c_str(), &width, &height, &channelCount, 4) }) {
    std::array<Image, 6> faceImgs;

    // 4:3
    if ((width - height) * 3 == height) {
      auto const faceSize{ width / 4 };

      if (!IsPowerOfTwo(faceSize)) {
        return nullptr;
      }

      return nullptr; // TODO
    }
    // 3:4
    else if ((height - width) * 3 == width) {
      auto const faceSize{ height / 4 };

      if (!IsPowerOfTwo(faceSize)) {
        return nullptr;
      }

      return nullptr; // TODO
    }
    // 6:1
    else if (width == 6 * height) {
      auto const faceSize{ width / 6 };

      if (!IsPowerOfTwo(faceSize)) {
        return nullptr;
      }

      for (int faceIdx = 0; faceIdx < 6; faceIdx++) {
        auto bytes{ std::make_unique_for_overwrite<u8[]>(faceSize * faceSize * 4) };

        for (int i = 0; i < faceSize; i++) {
          std::ranges::copy_n(data.get() + i * width * 4 + faceIdx * faceSize * 4, faceSize * 4, bytes.get() + i * faceSize * 4);
        }

        faceImgs[faceIdx] = Image{ static_cast<u32>(faceSize), static_cast<u32>(faceSize), 4, std::move(bytes) };
      }
    }
    // 1:6
    else if (height == 6 * width) {
      auto const faceSize{ height / 6 };

      if (!IsPowerOfTwo(faceSize)) {
        return nullptr;
      }

      for (int faceIdx = 0; faceIdx < 6; faceIdx++) {
        auto bytes{ std::make_unique_for_overwrite<u8[]>(faceSize * faceSize * 4) };
        std::ranges::copy_n(data.get() + faceIdx * faceSize * faceSize * 4, faceSize * faceSize * 4, bytes.get());
        faceImgs[faceIdx] = Image{ static_cast<u32>(faceSize), static_cast<u32>(faceSize), 4, std::move(bytes) };
      }
    } else {
      return nullptr;
    }

    // Create the cubemap from the face imgs
    return std::make_unique<Cubemap>(faceImgs);
  }

  return nullptr;
}


auto mage::CubemapLoader::GetPrecedence() const noexcept -> int {
  return 0;
}
}
