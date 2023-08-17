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
    .property("Color Texture (sRGB)", &sorcery::TextureImporter::mIsSrgb);
}


namespace sorcery {
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

  DirectX::ScratchImage finalImg;

  if (mTexType == TextureType::Texture2D) {
    if (mGenerateMips) {
      DirectX::ScratchImage mipChain;
      if (FAILED(GenerateMipMaps(*img.GetImage(0, 0, 0), DirectX::TEX_FILTER_DEFAULT, 0, mipChain))) {
        return false;
      }
      img = std::move(mipChain);
    }

    if (mAllowBlockCompression && !DirectX::IsCompressed(img.GetMetadata().format)) {
      DXGI_FORMAT compressionFormat;

      if (auto const typelessFormat{DirectX::MakeTypeless(img.GetMetadata().format)}; typelessFormat ==
                                                                                      DXGI_FORMAT_R8_TYPELESS) {
        compressionFormat = DXGI_FORMAT_BC4_UNORM;
      } else if (typelessFormat == DXGI_FORMAT_R8G8_TYPELESS) {
        compressionFormat = DXGI_FORMAT_BC5_UNORM;
      } else if (typelessFormat == DXGI_FORMAT_R8G8B8A8_TYPELESS || typelessFormat == DXGI_FORMAT_B8G8R8A8_TYPELESS) {
        compressionFormat = img.IsAlphaAllOpaque() ? DXGI_FORMAT_BC1_UNORM : DXGI_FORMAT_BC3_UNORM;
        if (DirectX::IsSRGB(img.GetMetadata().format)) {
          compressionFormat = DirectX::MakeSRGB(compressionFormat);
        }
      } else if (DirectX::FormatDataType(img.GetMetadata().format) == DirectX::FORMAT_TYPE_FLOAT) {
        compressionFormat = DXGI_FORMAT_BC6H_UF16;
      } else {
        return false;
      }

      DirectX::ScratchImage compressed;
      if (FAILED(
        Compress(img.GetImages(), img.GetImageCount(), img.GetMetadata(), compressionFormat, DirectX::
          TEX_COMPRESS_PARALLEL, DirectX::TEX_THRESHOLD_DEFAULT, compressed))) {
        return false;
      }
      img = std::move(compressed);
    }

    img.OverrideFormat(mIsSrgb
                         ? DirectX::MakeSRGB(img.GetMetadata().format)
                         : DirectX::MakeLinear(img.GetMetadata().format));

    DirectX::Blob blob;

    if (FAILED(SaveToDDSMemory(img.GetImages(), img.GetImageCount(), img.GetMetadata(), DirectX::DDS_FLAGS_NONE, blob))) {
      return false;
    }

    bytes.reserve(std::size(bytes) + blob.GetBufferSize());
    std::ranges::copy(std::span{static_cast<std::byte const*>(blob.GetBufferPointer()), blob.GetBufferSize()}, std::back_inserter(bytes));
    categ = ExternalResourceCategory::Texture;
    return true;
  }

  if (mTexType == TextureType::Cubemap) {
    auto const meta{img.GetMetadata()};

    if (meta.IsCubemap()) {
      // TODO
      return false;
    }

    if (img.GetImageCount() == 1 && meta.mipLevels == 1 && meta.arraySize == 1 && meta.depth == 1) {
      if (meta.width == 6 * meta.height) {
        // TODO
        return false;
      }

      if (6 * meta.width == meta.height) {
        std::array<DirectX::Image, 6> faceImgs;

        for (auto i{0}; i < 6; i++) {
          faceImgs[i].width = meta.width;
          faceImgs[i].height = meta.height / 6;
          faceImgs[i].format = meta.format;
          faceImgs[i].rowPitch = img.GetImage(0, 0, 0)->rowPitch;
          faceImgs[i].slicePitch = img.GetImage(0, 0, 0)->slicePitch;
          faceImgs[i].pixels = &img.GetPixels()[i * faceImgs[i].height * faceImgs[i].rowPitch];
        }

        DirectX::ScratchImage cube;

        if (FAILED(cube.InitializeCubeFromImages(faceImgs.data(), 6))) {
          return false;
        }

        cube.OverrideFormat(mIsSrgb
                              ? DirectX::MakeSRGB(cube.GetMetadata().format)
                              : DirectX::MakeLinear(cube.GetMetadata().format));

        if (mGenerateMips) {
          DirectX::ScratchImage mipChain;
          if (FAILED(DirectX::GenerateMipMaps(cube.GetImages(), cube.GetImageCount(), cube.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, mipChain))) {
            return false;
          }
          cube = std::move(mipChain);
        }

        if (mAllowBlockCompression && !DirectX::IsCompressed(cube.GetMetadata().format)) {
          DXGI_FORMAT compressionFormat;

          if (auto const typelessFormat{DirectX::MakeTypeless(cube.GetMetadata().format)}; typelessFormat ==
                                                                                           DXGI_FORMAT_R8_TYPELESS) {
            compressionFormat = DXGI_FORMAT_BC4_UNORM;
          } else if (typelessFormat == DXGI_FORMAT_R8G8_TYPELESS) {
            compressionFormat = DXGI_FORMAT_BC5_UNORM;
          } else if (typelessFormat == DXGI_FORMAT_R8G8B8A8_TYPELESS || typelessFormat == DXGI_FORMAT_B8G8R8A8_TYPELESS) {
            compressionFormat = cube.IsAlphaAllOpaque() ? DXGI_FORMAT_BC1_UNORM : DXGI_FORMAT_BC3_UNORM;
            if (DirectX::IsSRGB(cube.GetMetadata().format)) {
              compressionFormat = DirectX::MakeSRGB(compressionFormat);
            }
          } else if (DirectX::FormatDataType(cube.GetMetadata().format) == DirectX::FORMAT_TYPE_FLOAT) {
            compressionFormat = DXGI_FORMAT_BC6H_UF16;
          } else {
            return false;
          }

          DirectX::ScratchImage compressed;
          if (FAILED(
            Compress(cube.GetImages(), cube.GetImageCount(), cube.GetMetadata(), compressionFormat, DirectX::
              TEX_COMPRESS_PARALLEL, DirectX::TEX_THRESHOLD_DEFAULT, compressed))) {
            return false;
          }
          cube = std::move(compressed);
        }

        DirectX::Blob blob;

        if (FAILED(SaveToDDSMemory(cube.GetImages(), cube.GetImageCount(), cube.GetMetadata(), DirectX::DDS_FLAGS_NONE, blob))) {
          return false;
        }

        bytes.reserve(std::size(bytes) + blob.GetBufferSize());
        std::ranges::copy(std::span{static_cast<std::byte const*>(blob.GetBufferPointer()), blob.GetBufferSize()}, std::back_inserter(bytes));
        categ = ExternalResourceCategory::Texture;
        return true;
      }
    }
  }

  return false;
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
