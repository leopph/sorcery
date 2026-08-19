#include "resource_id.hpp"

#include <sstream>


namespace sorcery {
auto ResourceId::Invalid() noexcept -> ResourceId {
  return ResourceId{};
}


ResourceId::ResourceId(Guid const guid, int const idx_in_file) :
  guid_{guid},
  idx_in_file_{idx_in_file} {}


auto ResourceId::GetGuid() const noexcept -> Guid {
  return guid_;
}


auto ResourceId::GetIdxInFile() const noexcept -> int {
  return idx_in_file_;
}


auto ResourceId::IsValid() const noexcept -> bool {
  return guid_.IsValid() && idx_in_file_ >= 0;
}


ResourceId::operator std::string() const {
  std::stringstream str_stream;
  str_stream << std::string{guid_} << ":" << idx_in_file_;
  return str_stream.str();
}


auto ResourceId::ToString() const -> std::string {
  return std::string{*this};
}


auto operator<=>(ResourceId const& lhs, ResourceId const& rhs) noexcept -> std::strong_ordering {
  return lhs.GetGuid() <=> rhs.GetGuid() != std::strong_ordering::equivalent
           ? lhs.GetGuid() <=> rhs.GetGuid()
           : lhs.GetIdxInFile() <=> rhs.GetIdxInFile();
}


auto operator==(ResourceId const& lhs, ResourceId const& rhs) noexcept -> bool {
  return lhs.GetGuid() == rhs.GetGuid() && lhs.GetIdxInFile() == rhs.GetIdxInFile();
}
}
