#pragma once

#include "ResourceImporter.hpp"

#include <string_view>


namespace sorcery {
class NativeResourceImporter final : public ResourceImporter {
  RTTR_ENABLE(ResourceImporter)

public:
  auto GetSupportedFileExtensions(std::pmr::vector<std::string>& out) -> void override;
  [[nodiscard]] auto Import(std::filesystem::path const& src, std::vector<std::byte>& bytes) -> bool override;
  [[nodiscard]] auto GetImportedType(std::filesystem::path const& resPathAbs) noexcept -> rttr::type override;
  [[nodiscard]] auto IsNativeImporter() const noexcept -> bool override;
};
}
