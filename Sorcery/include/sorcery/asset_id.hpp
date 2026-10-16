#pragma once

#include <sorcery/Core.hpp>
#include <sorcery/Guid.hpp>


namespace sorcery {
class AssetId {
public:
  [[nodiscard]] LEOPPHAPI static auto Invalid() noexcept -> AssetId;

  AssetId() = default;
  LEOPPHAPI AssetId(Guid guid, int idx_in_file);

  [[nodiscard]] LEOPPHAPI auto GetGuid() const noexcept -> Guid;
  [[nodiscard]] LEOPPHAPI auto GetIdxInFile() const noexcept -> int;

  [[nodiscard]] LEOPPHAPI auto IsValid() const noexcept -> bool;

  [[nodiscard]] SORCERYAPI explicit operator std::string() const;
  [[nodiscard]] SORCERYAPI auto ToString() const -> std::string;

private:
  Guid guid_{Guid::Invalid()};
  int idx_in_file_{-1};
};


[[nodiscard]] LEOPPHAPI auto operator<=>(AssetId const& lhs, AssetId const& rhs) noexcept -> std::strong_ordering;
[[nodiscard]] LEOPPHAPI auto operator==(AssetId const& lhs, AssetId const& rhs) noexcept -> bool;
}
