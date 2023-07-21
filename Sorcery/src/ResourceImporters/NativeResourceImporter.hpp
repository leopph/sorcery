#pragma once

#include "ResourceImporter.hpp"


namespace sorcery {
class NativeResourceImporter final : public ResourceImporter {
  RTTR_ENABLE(ResourceImporter)

public:
  static char const* const MATERIAL_FILE_EXT;
  static char const* const SCENE_FILE_EXT;

  auto GetSupportedFileExtensions(std::vector<std::string>& out) -> void override;
  [[nodiscard]] auto Import(std::filesystem::path const& src) -> std::shared_ptr<Resource> override;
  [[nodiscard]] auto GetPrecedence() const noexcept -> int override;
};
}
