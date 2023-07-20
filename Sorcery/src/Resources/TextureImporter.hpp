#pragma once

#include "ResourceImporter.hpp"


namespace sorcery {
class TextureImporter final : public ResourceImporter {
public:
  enum class TextureType : int {
    Texture2D = 0,
    Cubemap   = 1
  };


  enum class NonPowerOfTwoRounding {
    None           = 0,
    NextPowerOfTwo = 1
  };

private:
  RTTR_ENABLE(ResourceImporter)
  RTTR_REGISTRATION_FRIEND

  TextureType mTexType{ TextureType::Texture2D };
  NonPowerOfTwoRounding mRounding{ NonPowerOfTwoRounding::NextPowerOfTwo };

public:
  LEOPPHAPI auto GetSupportedFileExtensions(std::vector<std::string>& out) -> void override;
  [[nodiscard]] LEOPPHAPI auto Import(std::filesystem::path const& src) -> std::shared_ptr<Resource> override;
  [[nodiscard]] LEOPPHAPI auto GetPrecedence() const noexcept -> int override;
};
}
