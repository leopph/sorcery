#include "sorcery/world_entity_group.hpp"


namespace sorcery {
WorldEntityGroup::WorldEntityGroup(std::uint32_t const value) noexcept :
  value_{value} {}


auto WorldEntityGroup::IsNone() const noexcept -> bool {
  return value_ == 0;
}


auto WorldEntityGroup::Value() const noexcept -> std::uint32_t {
  return value_;
}


WorldEntityGroup::operator std::uint32_t() const noexcept {
  return Value();
}
}
