#include "sorcery/host/rendering/view_history_key.hpp"

#include <atomic>


namespace sorcery::rendering {
auto ViewHistoryKey::ForObject(ObjectId const& id) noexcept -> ViewHistoryKey {
  // The generation is part of the key, so a new object in a reused slot never inherits old history.
  return ViewHistoryKey{Kind::kObject, static_cast<std::uint64_t>(id.gen) << 32 | id.idx};
}


auto ViewHistoryKey::MakeUnique() noexcept -> ViewHistoryKey {
  static std::atomic<std::uint64_t> next{0};
  return ViewHistoryKey{Kind::kUnique, next.fetch_add(1, std::memory_order_relaxed)};
}


auto ViewHistoryKey::IsNone() const noexcept -> bool {
  return kind_ == Kind::kNone;
}


auto ViewHistoryKey::Hash() const noexcept -> std::size_t {
  return std::hash<std::uint64_t>{}(value_) ^ (static_cast<std::size_t>(kind_) << 61);
}


ViewHistoryKey::ViewHistoryKey(Kind const kind, std::uint64_t const value) noexcept :
  kind_{kind},
  value_{value} {}
}
