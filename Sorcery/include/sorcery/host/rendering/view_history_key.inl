#pragma once


inline auto std::hash<sorcery::rendering::ViewHistoryKey>::operator()(
  sorcery::rendering::ViewHistoryKey const& key) const noexcept -> std::size_t {
  return key.Hash();
}
