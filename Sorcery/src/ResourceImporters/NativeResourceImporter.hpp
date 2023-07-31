#pragma once

#include "ResourceImporter.hpp"


namespace sorcery {
class NativeResourceImporter final : public ResourceImporter {
  RTTR_ENABLE(ResourceImporter)

public:
  static char const* const MATERIAL_FILE_EXT;
  static char const* const SCENE_FILE_EXT;

  LEOPPHAPI auto GetSupportedFileExtensions(std::vector<std::string>& out) -> void override;
  [[nodiscard]] LEOPPHAPI auto Import(std::filesystem::path const& src) -> ObserverPtr<Resource> override;
  [[nodiscard]] LEOPPHAPI auto GetPrecedence() const noexcept -> int override;
};
}
