#include "sorcery/asset_id.hpp"

#include <sstream>


namespace sorcery {
auto AssetId::Invalid() noexcept -> AssetId {
  return AssetId{};
}


AssetId::AssetId(Guid const guid, int const idx_in_file) :
  guid_{guid},
  idx_in_file_{idx_in_file} {}


auto AssetId::GetGuid() const noexcept -> Guid {
  return guid_;
}


auto AssetId::GetIdxInFile() const noexcept -> int {
  return idx_in_file_;
}


auto AssetId::IsValid() const noexcept -> bool {
  return guid_.IsValid() && idx_in_file_ >= 0;
}


AssetId::operator std::string() const {
  std::stringstream str_stream;
  str_stream << std::string{guid_} << ":" << idx_in_file_;
  return str_stream.str();
}


auto AssetId::ToString() const -> std::string {
  return std::string{*this};
}


auto operator<=>(AssetId const& lhs, AssetId const& rhs) noexcept -> std::strong_ordering {
  return lhs.GetGuid() <=> rhs.GetGuid() != std::strong_ordering::equivalent
           ? lhs.GetGuid() <=> rhs.GetGuid()
           : lhs.GetIdxInFile() <=> rhs.GetIdxInFile();
}


auto operator==(AssetId const& lhs, AssetId const& rhs) noexcept -> bool {
  return lhs.GetGuid() == rhs.GetGuid() && lhs.GetIdxInFile() == rhs.GetIdxInFile();
}
}
