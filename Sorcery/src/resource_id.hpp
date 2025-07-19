#pragma once

#include "Core.hpp"
#include "Guid.hpp"


namespace sorcery {
class ResourceId {
public:
  ResourceId() = default;
  LEOPPHAPI ResourceId(Guid guid, int idx_in_file);

  [[nodiscard]] LEOPPHAPI auto GetGuid() const noexcept -> Guid;
  [[nodiscard]] LEOPPHAPI auto GetIdxInFile() const noexcept -> int;

  [[nodiscard]] LEOPPHAPI auto IsValid() const noexcept -> bool;

private:
  Guid guid_{Guid::Invalid()};
  int idx_in_file_{-1};
};


[[nodiscard]] LEOPPHAPI auto operator<=>(ResourceId const& lhs, ResourceId const& rhs) noexcept -> std::strong_ordering;
[[nodiscard]] LEOPPHAPI auto operator==(ResourceId const& lhs, ResourceId const& rhs) noexcept -> bool;
}
