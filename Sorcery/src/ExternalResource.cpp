#include "ExternalResource.hpp"

#include "Serialization.hpp"


namespace sorcery {
auto PackExternalResource(ExternalResourceCategory const categ, std::span<std::byte const> resBytes, std::vector<std::byte>& fileBytes) noexcept -> void {
  fileBytes.reserve(sizeof(categ) + std::size(resBytes));
  SerializeToBinary(categ, fileBytes);
  std::ranges::copy(resBytes, std::back_inserter(fileBytes));
}


auto UnpackExternalResource(std::span<std::byte const> fileBytes, ExternalResourceCategory& categ, std::vector<std::byte>& resBytes) noexcept -> bool {
  if (std::size(fileBytes) < sizeof(categ)) {
    return false;
  }

  if (!DeserializeFromBinary(fileBytes, categ)) {
    return false;
  }

  std::ranges::copy(fileBytes.subspan<sizeof(categ)>(), std::back_inserter(resBytes));
  return true;
}
}
