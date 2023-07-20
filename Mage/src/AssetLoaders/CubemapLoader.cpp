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


auto mage::CubemapLoader::Load(std::filesystem::path const& src, [[maybe_unused]] std::filesystem::path const& cache) -> std::unique_ptr<Resource> { }


auto mage::CubemapLoader::GetPrecedence() const noexcept -> int {
  return 0;
}
}
