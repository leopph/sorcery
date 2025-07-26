#include "ExternalResource.hpp"

#include <type_traits>

#include "Serialization.hpp"


namespace sorcery {
auto PackExternalResource(ExternalResourceCategory const categ, std::span<std::byte const> resBytes,
                          std::vector<std::byte>& fileBytes) noexcept -> void {
  fileBytes.reserve(sizeof(categ) + std::size(resBytes));
  SerializeToBinary(categ, fileBytes);
  std::ranges::copy(resBytes, std::back_inserter(fileBytes));
}


auto UnpackExternalResource(
  std::span<std::byte const> const file_bytes) noexcept -> std::optional<UnpackedExternalResource> {
  if (std::size(file_bytes) < sizeof(ExternalResourceCategory)) {
    return std::nullopt;
  }

  UnpackedExternalResource ret;

  if (!DeserializeFromBinary(file_bytes, ret.category)) {
    return std::nullopt;
  }

  ret.bytes = file_bytes.subspan<sizeof(ExternalResourceCategory)>();
  return ret;
}
}
