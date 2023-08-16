#pragma once

#include "ResourceImporter.hpp"

#include <array>
#include <string_view>


namespace sorcery {
class TextureImporter final : public ResourceImporter {
  constexpr static std::string_view DDS_FILE_EXT{".dds"};
  constexpr static std::string_view HDR_FILE_EXT{".hdr"};
  constexpr static std::string_view TGA_FILE_EXT{".tga"};
  constexpr static std::array<std::string_view, 6> WIC_FILE_EXTS{".bmp", ".png", ".gif", ".tiff", ".jpeg", ".jpg"};

public:
  enum class TextureType : int {
    Texture2D = 0,
    Cubemap   = 1
  };

private:
  RTTR_ENABLE(ResourceImporter)
  RTTR_REGISTRATION_FRIEND

  TextureType mTexType{TextureType::Texture2D};
  bool mKeepInCpuMemory{false};
  bool mAllowBlockCompression{true};
  bool mIsSrgb{true};
  bool mGenerateMips{true};

public:
  auto GetSupportedFileExtensions(std::pmr::vector<std::string>& out) -> void override;
  [[nodiscard]] auto Import(std::filesystem::path const& src, std::vector<std::byte>& bytes) -> bool override;
  [[nodiscard]] auto GetImportedType(std::filesystem::path const& resPathAbs) noexcept -> rttr::type override;
};
}
