#pragma once

#include "ResourceImporter.hpp"


namespace sorcery {
class TextureImporter final : public ResourceImporter {
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

public:
  LEOPPHAPI auto GetSupportedFileExtensions(std::pmr::vector<std::string>& out) -> void override;
  [[nodiscard]] LEOPPHAPI auto Import(std::filesystem::path const& src) -> ObserverPtr<Resource> override;
  [[nodiscard]] LEOPPHAPI auto GetImportedType(std::filesystem::path const& resPathAbs) noexcept -> rttr::type override;
};
}
