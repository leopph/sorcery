#include "TextureImporter.hpp"
#include "../Resources/Texture2D.hpp"
#include "../Resources/Cubemap.hpp"
#include "../FileIo.hpp"

#include <DirectXTex.h>

#include <algorithm>
#include <cassert>

RTTR_REGISTRATION {
  rttr::registration::enumeration<sorcery::TextureImporter::TextureType>("Texture Import Type")(
    rttr::value("Texture2D", sorcery::TextureImporter::TextureType::Texture2D),
    rttr::value("Cubemap", sorcery::TextureImporter::TextureType::Cubemap)
  );

  rttr::registration::class_<sorcery::TextureImporter>("Texture Importer")
    .REFLECT_REGISTER_RESOURCE_IMPORTER_CTOR
    .property("Texture Type", &sorcery::TextureImporter::mTexType)
    .property("Keep in CPU Memory", &sorcery::TextureImporter::mKeepInCpuMemory)
    .property("Allow Block Compression", &sorcery::TextureImporter::mAllowBlockCompression)
    .property("Color Texture (sRGB)", &sorcery::TextureImporter::mIsSrgb)
    .property("Generate Mipmaps", &sorcery::TextureImporter::mGenerateMips);
}


namespace sorcery {
namespace {
[[nodiscard]] auto CompressTexture(DirectX::ScratchImage const& src, bool const isSrgb, DirectX::ScratchImage& out) noexcept -> bool {
  DXGI_FORMAT compressionFormat;

  auto const getFormatChannelCount{
    [](DXGI_FORMAT const format) {
      return DirectX::BitsPerPixel(format) / DirectX::BitsPerColor(format);
    }
  };

  if (DirectX::FormatDataType(src.GetMetadata().format) == DirectX::FORMAT_TYPE_FLOAT) {
    compressionFormat = DXGI_FORMAT_BC6H_UF16;
  } else if (auto const channelCount{getFormatChannelCount(src.GetMetadata().format)}; channelCount == 1) {
    compressionFormat = DXGI_FORMAT_BC4_UNORM;
  } else if (channelCount == 2) {
    compressionFormat = DXGI_FORMAT_BC5_UNORM;
  } else if (channelCount == 4) {
    compressionFormat = src.IsAlphaAllOpaque() ? DXGI_FORMAT_BC1_UNORM : DXGI_FORMAT_BC3_UNORM;
  } else {
    return false;
  }

  DirectX::ScratchImage compressed;

  if (FAILED(Compress(src.GetImages(), src.GetImageCount(), src.GetMetadata(), compressionFormat, DirectX::TEX_COMPRESS_PARALLEL | (isSrgb ? DirectX::TEX_COMPRESS_SRGB : DirectX::TEX_COMPRESS_DEFAULT), DirectX::TEX_THRESHOLD_DEFAULT, compressed))) {
    return false;
  }

  out = std::move(compressed);
  return true;
}
}


auto TextureImporter::GetSupportedFileExtensions(std::pmr::vector<std::string>& out) -> void {
  out.emplace_back(DDS_FILE_EXT);
  out.emplace_back(HDR_FILE_EXT);
  out.emplace_back(TGA_FILE_EXT);
  std::ranges::transform(WIC_FILE_EXTS, std::back_inserter(out), [](std::string_view const sv) {
    return std::string{sv};
  });
}


auto TextureImporter::Import(std::filesystem::path const& src, std::vector<std::byte>& bytes, ExternalResourceCategory& categ) -> bool {
  std::vector<unsigned char> fileBytes;

  if (!ReadFileBinary(src, fileBytes)) {
    return false;
  }

  if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))) {
    return false;
  }

  DirectX::ScratchImage img;

  // Parse image file bytes

  HRESULT hr;

  if (src.extension() == ".dds") {
    hr = LoadFromDDSMemory(fileBytes.data(), fileBytes.size(), DirectX::DDS_FLAGS_NONE, nullptr, img);
  } else if (src.extension() == ".hdr") {
    hr = LoadFromHDRMemory(fileBytes.data(), fileBytes.size(), nullptr, img);
  } else if (src.extension() == ".tga") {
    hr = LoadFromTGAMemory(fileBytes.data(), fileBytes.size(), nullptr, img);
  } else if (std::ranges::any_of(std::array{".bmp", ".png", ".gif", ".tiff", ".jpeg", ".jpg"},
    [&src](char const* const ext) {
      return src.extension() == ext;
    })) {
    hr = LoadFromWICMemory(fileBytes.data(), fileBytes.size(), DirectX::WIC_FLAGS_NONE, nullptr, img);
  } else {
    return false;
  }

  if (FAILED(hr)) {
    return false;
  }

  // Extract first 2D image if necessary
  if (mTexType == TextureType::Texture2D && (img.GetMetadata().IsCubemap() || img.GetMetadata().arraySize != 1)) {
    DirectX::ScratchImage extracted;

    if (FAILED(extracted.InitializeFromImage(*img.GetImage(0, 0, 0)))) {
      return false;
    }

    img = std::move(extracted);
  }

  // Assemble cubemap if necessary
  if (mTexType == TextureType::Cubemap && !img.GetMetadata().IsCubemap()) {
    if (auto const meta{img.GetMetadata()}; !meta.IsCubemap()) {
      if (img.GetImageCount() == 1 && meta.mipLevels == 1 && meta.arraySize == 1 && meta.depth == 1) {
        std::array<DirectX::Image, 6> faceImgs;

        if (meta.width == 6 * meta.height) {
          for (auto i{0}; i < 6; i++) {
            faceImgs[i].width = meta.width / 6;
            faceImgs[i].height = meta.height;
            faceImgs[i].format = meta.format;
            faceImgs[i].rowPitch = img.GetImage(0, 0, 0)->rowPitch;
            faceImgs[i].slicePitch = img.GetImage(0, 0, 0)->slicePitch;
            faceImgs[i].pixels = &img.GetPixels()[i * faceImgs[i].width * DirectX::BitsPerPixel(faceImgs[i].format) / 8];
          }
        } else if (6 * meta.width == meta.height) {
          for (auto i{0}; i < 6; i++) {
            faceImgs[i].width = meta.width;
            faceImgs[i].height = meta.height / 6;
            faceImgs[i].format = meta.format;
            faceImgs[i].rowPitch = img.GetImage(0, 0, 0)->rowPitch;
            faceImgs[i].slicePitch = img.GetImage(0, 0, 0)->slicePitch;
            faceImgs[i].pixels = &img.GetPixels()[i * faceImgs[i].height * faceImgs[i].rowPitch];
          }
        } else {
          return false;
        }

        DirectX::ScratchImage cube;

        if (FAILED(cube.InitializeCubeFromImages(faceImgs.data(), 6))) {
          return false;
        }

        img = std::move(cube);
      } else {
        // TODO
        return false;
      }
    }
  }

  // Generate cubemaps if necessary
  if (mGenerateMips && img.GetMetadata().mipLevels == 1) {
    DirectX::ScratchImage mipChain;

    if (FAILED(GenerateMipMaps(img.GetImages(), img.GetImageCount(), img.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, mipChain))) {
      return false;
    }

    img = std::move(mipChain);
  }

  // Compress if allowed
  if (mAllowBlockCompression && !DirectX::IsCompressed(img.GetMetadata().format)) {
    DirectX::ScratchImage compressed;

    if (!CompressTexture(img, mIsSrgb, compressed)) {
      return false;
    }

    img = std::move(compressed);
  }

  // Mark image as linear or sRGB based on user setting
  img.OverrideFormat(mIsSrgb ? DirectX::MakeSRGB(img.GetMetadata().format) : DirectX::MakeLinear(img.GetMetadata().format));

  // Save processed image

  DirectX::Blob blob;

  if (FAILED(SaveToDDSMemory(img.GetImages(), img.GetImageCount(), img.GetMetadata(), DirectX::DDS_FLAGS_NONE, blob))) {
    return false;
  }

  bytes.reserve(std::size(bytes) + blob.GetBufferSize());
  std::ranges::copy(std::span{static_cast<std::byte const*>(blob.GetBufferPointer()), blob.GetBufferSize()}, std::back_inserter(bytes));
  categ = ExternalResourceCategory::Texture;
  return true;
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
