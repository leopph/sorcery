#pragma once

#include <utility>


namespace sorcery {
template<typename... Args>
auto MakeUniqueObject(rttr::type const& type, Args&&... args) -> std::unique_ptr<Object> {
  if (type.is_derived_from(rttr::type::get<Object>())) {
    return std::unique_ptr<Object>{type.create(std::forward<Args>(args)...).template get_value<Object*>()};
  }

  return nullptr;
}
}
