#include "Texture2DLoader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Texture2D.hpp"

#include <array>


namespace sorcery::mage {
auto Texture2DLoader::GetSupportedExtensions() const -> std::span<std::string const> {
  static std::array<std::string, 9> const extensions{
    "png", "jpg", "jpeg", "tga", "bmp", "psd", "gif", "hdr", "pic"
  };

  return extensions;
}


auto Texture2DLoader::Load(std::filesystem::path const& src, [[maybe_unused]] std::filesystem::path const& cache) -> std::unique_ptr<Object> {
  int width;
  int height;
  int channelCount;

  if (auto const imgData{ stbi_load(src.string().c_str(), &width, &height, &channelCount, 4) }) {
    return std::make_unique<Texture2D>(Image{ static_cast<u32>(width), static_cast<u32>(height), 4, std::unique_ptr<u8[]>{ imgData } });
  }

  return nullptr;
}


auto Texture2DLoader::GetPrecedence() const noexcept -> int {
  return 0;
}
}
