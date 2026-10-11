#pragma once

#include <type_traits>


namespace sorcery {
template<typename To, typename From>
auto ReflCast(From obj) noexcept -> To {
  if constexpr (std::is_pointer_v<From> && std::is_pointer_v<To>) {
    if (!obj) {
      return nullptr;
    }
  }

  return rttr::rttr_cast<To>(obj);
}
}
