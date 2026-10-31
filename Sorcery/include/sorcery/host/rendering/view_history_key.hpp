#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>
#include <functional>

#include <sorcery/Core.hpp>
#include <sorcery/object_id.hpp>


namespace sorcery::rendering {
class ViewHistoryKey {
public:
  ViewHistoryKey() = default;

  [[nodiscard]] SORCERYAPI static
  auto ForObject(ObjectId const& id) noexcept -> ViewHistoryKey;

  [[nodiscard]] SORCERYAPI static
  auto MakeUnique() noexcept -> ViewHistoryKey;

  [[nodiscard]] SORCERYAPI
  auto IsNone() const noexcept -> bool;

  [[nodiscard]] SORCERYAPI
  auto Hash() const noexcept -> std::size_t;

  auto operator<=>(ViewHistoryKey const& other) const noexcept = default;

private:
  enum class Kind : std::uint8_t {
    kNone   = 0,
    kObject = 1,
    kUnique = 2
  };


  ViewHistoryKey(Kind kind, std::uint64_t value) noexcept;

  Kind kind_{Kind::kNone};
  std::uint64_t value_{0};
};
}


template<>
struct std::hash<sorcery::rendering::ViewHistoryKey> {
  [[nodiscard]] auto operator()(sorcery::rendering::ViewHistoryKey const& key) const noexcept -> std::size_t;
};


#include <sorcery/host/rendering/view_history_key.inl>
