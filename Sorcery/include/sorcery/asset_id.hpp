#pragma once

#include <cstdint>
#include <optional>

#include <sorcery/Core.hpp>
#include <sorcery/Guid.hpp>


namespace sorcery {
class AssetId {
public:
  [[nodiscard]] LEOPPHAPI static auto Invalid() noexcept -> AssetId;

  AssetId() = default;
  LEOPPHAPI AssetId(Guid guid, std::uint64_t id_in_file);

  [[nodiscard]] LEOPPHAPI auto GetGuid() const noexcept -> Guid;
  [[nodiscard]] LEOPPHAPI auto GetIdInFile() const noexcept -> std::optional<uint64_t>;

  [[nodiscard]] LEOPPHAPI auto IsValid() const noexcept -> bool;

  [[nodiscard]] SORCERYAPI explicit operator std::string() const;
  [[nodiscard]] SORCERYAPI auto ToString() const -> std::string;

private:
  Guid guid_{Guid::Invalid()};
  std::optional<std::uint64_t> id_in_file_;
};


[[nodiscard]] LEOPPHAPI auto operator<=>(AssetId const& lhs, AssetId const& rhs) noexcept -> std::strong_ordering;
[[nodiscard]] LEOPPHAPI auto operator==(AssetId const& lhs, AssetId const& rhs) noexcept -> bool;
}
