#pragma once


inline auto std::hash<sorcery::WorldEntityGroup>::operator()(
  sorcery::WorldEntityGroup const& group) const noexcept -> std::size_t {
  return std::hash<std::uint32_t>{}(group.Value());
}
