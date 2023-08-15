#include "ResourceBinary.hpp"

#include "Serialization.hpp"


namespace sorcery {
auto PackResourceBinary(Resource::Category const categ, std::span<std::uint8_t const> const resBytes, std::vector<std::uint8_t>& fileBytes) noexcept -> void {
  fileBytes.reserve(sizeof(categ) + std::size(resBytes));
  SerializeToBinary(categ, fileBytes);
  std::ranges::copy(resBytes, std::back_inserter(fileBytes));
}


auto UnpackResourceBinary(std::span<std::uint8_t const> const fileBytes, Resource::Category& categ, std::vector<std::uint8_t>& resBytes) noexcept -> bool {
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
