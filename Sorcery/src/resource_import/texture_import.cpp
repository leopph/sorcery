#include "texture_import.hpp"

#include <algorithm>
#include <cassert>

#include <DirectXTex.h>

#include "../Resources/Cubemap.hpp"
#include "../resources/Texture2D.hpp"

RTTR_REGISTRATION {
  rttr::registration::enumeration<sorcery::TextureImportType>("Texture Import Type")(
    rttr::value("Texture2D", sorcery::TextureImportType::kTexture2D),
    rttr::value("Cubemap", sorcery::TextureImportType::kCubemap)
  );

  rttr::registration::class_<sorcery::TextureImportSettings>("Texture Import Settings")
    .constructor<>()(rttr::policy::ctor::as_object)
    .property("Texture Type", &sorcery::TextureImportSettings::type)
    .property("Allow Block Compression", &sorcery::TextureImportSettings::allow_block_compression)
    .property("Color Texture (sRGB)", &sorcery::TextureImportSettings::is_srgb)
    .property("Generate Mipmaps", &sorcery::TextureImportSettings::generate_mips);
}


namespace sorcery {
namespace {
[[nodiscard]]
auto CompressTexture(
  DirectX::ScratchImage const& src, bool const is_srgb,
  DirectX::ScratchImage& out
) noexcept -> bool {
  DXGI_FORMAT compression_format;

  auto const getFormatChannelCount{
    [](DXGI_FORMAT const format) {
      return DirectX::BitsPerPixel(format) / DirectX::BitsPerColor(format);
    }
  };

  if (DirectX::FormatDataType(src.GetMetadata().format) == DirectX::FORMAT_TYPE_FLOAT) {
    compression_format = DXGI_FORMAT_BC6H_UF16;
  } else if (auto const channel_count{getFormatChannelCount(src.GetMetadata().format)}; channel_count == 1) {
    compression_format = DXGI_FORMAT_BC4_UNORM;
  } else if (channel_count == 2) {
    compression_format = DXGI_FORMAT_BC5_UNORM;
  } else if (channel_count == 4) {
    compression_format = src.IsAlphaAllOpaque() ? DXGI_FORMAT_BC1_UNORM : DXGI_FORMAT_BC3_UNORM;
  } else {
    return false;
  }

  DirectX::ScratchImage compressed;

  if (FAILED(
    Compress(src.GetImages(), src.GetImageCount(), src.GetMetadata(), compression_format, DirectX::TEX_COMPRESS_PARALLEL
      | (is_srgb ? DirectX::TEX_COMPRESS_SRGB : DirectX::TEX_COMPRESS_DEFAULT), DirectX::TEX_THRESHOLD_DEFAULT,
      compressed))) {
    return false;
  }

  out = std::move(compressed);
  return true;
}
}


auto ImportTexture(
  TextureImportSource const& src,
  TextureImportSettings const& settings
) -> std::optional<TextureImportResult> {
  DirectX::ScratchImage img;

  // Parse image file bytes

  HRESULT hr;

  if (src.ext == u8".dds") {
    hr = LoadFromDDSMemory(src.file_bytes.data(), src.file_bytes.size(), DirectX::DDS_FLAGS_NONE, nullptr, img);
  } else if (src.ext == u8".hdr") {
    hr = LoadFromHDRMemory(src.file_bytes.data(), src.file_bytes.size(), nullptr, img);
  } else if (src.ext == u8".tga") {
    hr = LoadFromTGAMemory(reinterpret_cast<std::uint8_t const*>(src.file_bytes.data()), src.file_bytes.size(), nullptr,
      img);
  } else if (std::ranges::any_of(std::array{u8".bmp", u8".png", u8".gif", u8".tiff", u8".jpeg", u8".jpg"},
    [&src](char8_t const* const ext) {
      return src.ext == ext;
    })) {
    hr = LoadFromWICMemory(src.file_bytes.data(), src.file_bytes.size(), DirectX::WIC_FLAGS_NONE, nullptr, img);
  } else {
    return std::nullopt;
  }

  if (FAILED(hr)) {
    return std::nullopt;
  }

  // Extract first 2D image if necessary
  if (settings.type == TextureImportType::kTexture2D && (
        img.GetMetadata().IsCubemap() || img.GetMetadata().arraySize != 1)) {
    DirectX::ScratchImage extracted;

    if (FAILED(extracted.InitializeFromImage(*img.GetImage(0, 0, 0)))) {
      return std::nullopt;
    }

    img = std::move(extracted);
  }

  // Assemble cubemap if necessary
  if (settings.type == TextureImportType::kCubemap && !img.GetMetadata().IsCubemap()) {
    if (auto const meta{img.GetMetadata()}; !meta.IsCubemap()) {
      if (img.GetImageCount() == 1 && meta.mipLevels == 1 && meta.arraySize == 1 && meta.depth == 1) {
        std::array<DirectX::Image, 6> faceImgs;
        auto const bytesPerPixel{DirectX::BitsPerPixel(meta.format) / 8};

        // [+X, -X, +Y, -Y, +Z, -Z]
        if (meta.width == 6 * meta.height) {
          for (auto i{0}; i < 6; i++) {
            faceImgs[i].width = meta.width / 6;
            faceImgs[i].height = meta.height;
            faceImgs[i].format = meta.format;
            faceImgs[i].rowPitch = img.GetImage(0, 0, 0)->rowPitch;
            faceImgs[i].slicePitch = img.GetImage(0, 0, 0)->slicePitch;
            faceImgs[i].pixels = &img.GetPixels()[i * faceImgs[i].width * bytesPerPixel];
          }
          // [+X, -X, +Y, -Y, +Z, -Z] ^ T
        } else if (6 * meta.width == meta.height) {
          for (auto i{0}; i < 6; i++) {
            faceImgs[i].width = meta.width;
            faceImgs[i].height = meta.height / 6;
            faceImgs[i].format = meta.format;
            faceImgs[i].rowPitch = img.GetImage(0, 0, 0)->rowPitch;
            faceImgs[i].slicePitch = img.GetImage(0, 0, 0)->slicePitch;
            faceImgs[i].pixels = &img.GetPixels()[i * faceImgs[i].height * faceImgs[i].rowPitch];
          }
          //     [+Y]
          // [-X, +Z, +X, -Z]
          //     [-Y]
        } else if (meta.width * 3 == meta.height * 4) {
          auto const faceSize{meta.width / 4};

          for (auto i{0}; i < 6; i++) {
            faceImgs[i].width = faceSize;
            faceImgs[i].height = faceSize;
            faceImgs[i].format = meta.format;
            faceImgs[i].rowPitch = img.GetImage(0, 0, 0)->rowPitch;
            faceImgs[i].slicePitch = img.GetImage(0, 0, 0)->slicePitch;
          }

          faceImgs[0].pixels = &img.GetPixels()[faceSize * faceImgs[0].rowPitch + 2 * faceSize * bytesPerPixel];
          faceImgs[1].pixels = &img.GetPixels()[faceSize * faceImgs[0].rowPitch];
          faceImgs[2].pixels = &img.GetPixels()[faceSize * bytesPerPixel];
          faceImgs[3].pixels = &img.GetPixels()[2 * faceSize * faceImgs[0].rowPitch + faceSize * bytesPerPixel];
          faceImgs[4].pixels = &img.GetPixels()[faceSize * faceImgs[0].rowPitch + faceSize * bytesPerPixel];
          faceImgs[5].pixels = &img.GetPixels()[faceSize * faceImgs[0].rowPitch + 3 * faceSize * bytesPerPixel];
          //     [+Y]
          // [-X, +Z, +X]
          //     [-Y]
          //     [-Z]
        } else if (meta.width * 4 == meta.height * 3) {
          auto const faceSize{meta.width / 3};

          for (auto i{0}; i < 6; i++) {
            faceImgs[i].width = faceSize;
            faceImgs[i].height = faceSize;
            faceImgs[i].format = meta.format;
            faceImgs[i].rowPitch = img.GetImage(0, 0, 0)->rowPitch;
            faceImgs[i].slicePitch = img.GetImage(0, 0, 0)->slicePitch;
          }

          faceImgs[0].pixels = &img.GetPixels()[faceSize * faceImgs[0].rowPitch + 2 * faceSize * bytesPerPixel];
          faceImgs[1].pixels = &img.GetPixels()[faceSize * faceImgs[0].rowPitch];
          faceImgs[2].pixels = &img.GetPixels()[faceSize * bytesPerPixel];
          faceImgs[3].pixels = &img.GetPixels()[2 * faceSize * faceImgs[0].rowPitch + faceSize * bytesPerPixel];
          faceImgs[4].pixels = &img.GetPixels()[faceSize * faceImgs[0].rowPitch + faceSize * bytesPerPixel];
          faceImgs[5].pixels = &img.GetPixels()[3 * faceSize * faceImgs[0].rowPitch + faceSize * bytesPerPixel];
        } else {
          return std::nullopt;
        }

        DirectX::ScratchImage cube;

        if (FAILED(cube.InitializeCubeFromImages(faceImgs.data(), 6))) {
          return std::nullopt;
        }

        img = std::move(cube);
      } else {
        // TODO
        return std::nullopt;
      }
    }
  }

  // Generate cubemaps if necessary
  if (settings.generate_mips && img.GetMetadata().mipLevels == 1) {
    DirectX::ScratchImage mipChain;

    if (FAILED(
      GenerateMipMaps(img.GetImages(), img.GetImageCount(), img.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, mipChain
      ))) {
      return std::nullopt;
    }

    img = std::move(mipChain);
  }

  // Compress if allowed
  if (settings.allow_block_compression && !DirectX::IsCompressed(img.GetMetadata().format)) {
    DirectX::ScratchImage compressed;

    if (!CompressTexture(img, settings.is_srgb, compressed)) {
      return std::nullopt;
    }

    img = std::move(compressed);
  }

  // Mark image as linear or sRGB based on user setting
  img.OverrideFormat(settings.is_srgb
                       ? DirectX::MakeSRGB(img.GetMetadata().format)
                       : DirectX::MakeLinear(img.GetMetadata().format));

  // Save processed image

  DirectX::Blob blob;

  if (FAILED(SaveToDDSMemory(img.GetImages(), img.GetImageCount(), img.GetMetadata(), DirectX::DDS_FLAGS_NONE, blob))) {
    return std::nullopt;
  }

  std::vector<std::byte> bytes;
  bytes.reserve(std::size(bytes) + blob.GetBufferSize());
  std::ranges::copy(std::span{std::bit_cast<std::byte const*>(blob.GetBufferPointer()), blob.GetBufferSize()},
    std::back_inserter(bytes));

  auto const imported_type{
    settings.type == TextureImportType::kTexture2D
      ? rttr::type::get<Texture2D>()
      : settings.type == TextureImportType::kCubemap
          ? rttr::type::get<Cubemap>()
          : rttr::type::get_by_name("")
  };

  return TextureImportResult{std::move(bytes), imported_type, ResourcePackagePayloadKind::kTexture};
}
}
