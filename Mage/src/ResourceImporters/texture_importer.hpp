#pragma once

#include <array>
#include <string_view>

#include "resource_importer.hpp"
#include "texture_import.hpp"


namespace sorcery::mage {
class TextureImporter final : public ResourceImporter {
  RTTR_ENABLE(ResourceImporter)
  RTTR_REGISTRATION_FRIEND

public:
  auto GetSupportedFileExtensions(
    std::pmr::vector<std::string>& out
  ) -> void override;

  [[nodiscard]]
  auto Import(
    std::filesystem::path const& src,
    std::vector<ResourceImportResult>& results
  ) -> bool override;

  [[nodiscard]]
  auto GetSettings() const -> TextureImportSettings const&;

  auto SetSettings(TextureImportSettings const& settings) -> void;

private:
  TextureImportSettings settings_;

  constexpr static std::string_view kDdsFileExt{".dds"};
  constexpr static std::string_view kHdrFileExt{".hdr"};
  constexpr static std::string_view kTgaFileExt{".tga"};
  constexpr static std::array<std::string_view, 6> kWicFileExts{".bmp", ".png", ".gif", ".tiff", ".jpeg", ".jpg"};
};
}
