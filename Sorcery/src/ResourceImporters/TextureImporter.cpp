#include "TextureImporter.hpp"
#include "../Resources/Texture2D.hpp"
#include "../Resources/Cubemap.hpp"

#include <stb_image.h>

RTTR_REGISTRATION {
  rttr::registration::enumeration<sorcery::TextureImporter::TextureType>("Texture Import Type")(
    rttr::value("Texture2D", sorcery::TextureImporter::TextureType::Texture2D),
    rttr::value("Cubemap", sorcery::TextureImporter::TextureType::Cubemap)
  );

  rttr::registration::enumeration<sorcery::TextureImporter::NonPowerOfTwoRounding>("Texture Import Non-Power Of Two Rounding")(
    rttr::value("None", sorcery::TextureImporter::NonPowerOfTwoRounding::None),
    rttr::value("NextPowerOfTwo", sorcery::TextureImporter::NonPowerOfTwoRounding::NextPowerOfTwo)
  );

  rttr::registration::class_<sorcery::TextureImporter>("Texture Importer")
    .REFLECT_REGISTER_RESOURCE_IMPORTER_CTOR
    .property("Texture Type", &sorcery::TextureImporter::mTexType)
    .property("Non-Power Of Two Rounding", &sorcery::TextureImporter::mRounding);
}


namespace sorcery {
auto TextureImporter::GetSupportedFileExtensions(std::vector<std::string>& out) -> void {
  out.emplace_back(".png");
  out.emplace_back(".jpg");
  out.emplace_back(".jpeg");
  out.emplace_back(".tga");
  out.emplace_back(".bmp");
  out.emplace_back(".psd");
  out.emplace_back(".gif");
  out.emplace_back(".hdr");
  out.emplace_back(".pic");
}


auto TextureImporter::Import(std::filesystem::path const& src) -> ObserverPtr<Resource> {
  switch (mTexType) {
    case TextureType::Texture2D: {
      int width;
      int height;
      int channelCount;

      if (auto const imgData{stbi_load(src.string().c_str(), &width, &height, &channelCount, 4)}) {
        return new Texture2D{Image{static_cast<u32>(width), static_cast<u32>(height), 4, std::unique_ptr<u8[]>{imgData}}};
      }

      return nullptr;
    }

    case TextureType::Cubemap: {
      int width;
      int height;
      int channelCount;

      if (std::unique_ptr<u8, decltype([](u8* const p) {
        stbi_image_free(p);
      })> const data{stbi_load(src.string().c_str(), &width, &height, &channelCount, 4)}) {
        std::array<Image, 6> faceImgs;

        // 4:3
        if ((width - height) * 3 == height) {
          auto const faceSize{width / 4};

          if (!IsPowerOfTwo(faceSize)) {
            return nullptr;
          }

          return nullptr; // TODO
        }
        // 3:4
        else if ((height - width) * 3 == width) {
          auto const faceSize{height / 4};

          if (!IsPowerOfTwo(faceSize)) {
            return nullptr;
          }

          return nullptr; // TODO
        }
        // 6:1
        else if (width == 6 * height) {
          auto const faceSize{width / 6};

          if (!IsPowerOfTwo(faceSize)) {
            return nullptr;
          }

          for (int faceIdx = 0; faceIdx < 6; faceIdx++) {
            auto bytes{std::make_unique_for_overwrite<u8[]>(faceSize * faceSize * 4)};

            for (int i = 0; i < faceSize; i++) {
              std::ranges::copy_n(data.get() + i * width * 4 + faceIdx * faceSize * 4, faceSize * 4, bytes.get() + i * faceSize * 4);
            }

            faceImgs[faceIdx] = Image{static_cast<u32>(faceSize), static_cast<u32>(faceSize), 4, std::move(bytes)};
          }
        }
        // 1:6
        else if (height == 6 * width) {
          auto const faceSize{height / 6};

          if (!IsPowerOfTwo(faceSize)) {
            return nullptr;
          }

          for (int faceIdx = 0; faceIdx < 6; faceIdx++) {
            auto bytes{std::make_unique_for_overwrite<u8[]>(faceSize * faceSize * 4)};
            std::ranges::copy_n(data.get() + faceIdx * faceSize * faceSize * 4, faceSize * faceSize * 4, bytes.get());
            faceImgs[faceIdx] = Image{static_cast<u32>(faceSize), static_cast<u32>(faceSize), 4, std::move(bytes)};
          }
        } else {
          return nullptr;
        }

        // Create the cubemap from the face imgs
        return new Cubemap{faceImgs};
      }

      return nullptr;
    }
  }

  return nullptr;
}


auto TextureImporter::GetPrecedence() const noexcept -> int {
  return 0;
}


auto TextureImporter::GetImportedType(std::filesystem::path const& resPathAbs) noexcept -> rttr::type {
  switch (mTexType) {
    case TextureType::Texture2D: {
      return rttr::type::get<Texture2D>();
    }
    case TextureType::Cubemap: {
      return rttr::type::get<Cubemap>();
    }
  }
  return rttr::type::get_by_name("");
}
}
