#include "sorcery/asset_id.hpp"

#include <sstream>


namespace sorcery {
auto AssetId::Invalid() noexcept -> AssetId {
  return AssetId{};
}


AssetId::AssetId(Guid const guid, std::uint64_t const id_in_file) :
  guid_{guid},
  id_in_file_{id_in_file} {}


auto AssetId::GetGuid() const noexcept -> Guid {
  return guid_;
}


auto AssetId::GetIdInFile() const noexcept -> std::optional<uint64_t> {
  return id_in_file_;
}


auto AssetId::IsValid() const noexcept -> bool {
  return guid_.IsValid() && id_in_file_.has_value();
}


AssetId::operator std::string() const {
  std::stringstream str_stream;
  str_stream << std::string{guid_} << ":";

  if (id_in_file_) {
    str_stream << *id_in_file_;
  } else {
    str_stream << "-1"; // -1 as backwards compatibility
  }

  return str_stream.str();
}


auto AssetId::ToString() const -> std::string {
  return std::string{*this};
}


auto operator<=>(AssetId const& lhs, AssetId const& rhs) noexcept -> std::strong_ordering {
  return lhs.GetGuid() <=> rhs.GetGuid() != std::strong_ordering::equivalent
           ? lhs.GetGuid() <=> rhs.GetGuid()
           : lhs.GetIdInFile() <=> rhs.GetIdInFile();
}


auto operator==(AssetId const& lhs, AssetId const& rhs) noexcept -> bool {
  return lhs.GetGuid() == rhs.GetGuid() && lhs.GetIdInFile() == rhs.GetIdInFile();
}
}
