#pragma once

#include "ResourceImporter.hpp"

#include <string_view>


namespace sorcery {
class NativeResourceImporter final : public ResourceImporter {
  RTTR_ENABLE(ResourceImporter)

public:
  constexpr static std::string_view MATERIAL_FILE_EXT{".mtl"};
  constexpr static std::string_view SCENE_FILE_EXT{".scene"};

  auto GetSupportedFileExtensions(std::vector<std::string>& out) -> void override;
  [[nodiscard]] auto Import(std::filesystem::path const& src, std::vector<std::uint8_t>& bytes) -> bool override;
  [[nodiscard]] auto GetImportedType(std::filesystem::path const& resPathAbs) noexcept -> rttr::type override;
};
}
