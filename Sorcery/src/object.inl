#pragma once

#include <utility>


namespace sorcery {
template<std::derived_from<Object> T>
auto Object::FindObjectOfType() -> T* {
  auto const all_objs{sAllObjects.LockShared()};

  if constexpr (std::same_as<Object, T>) {
    return all_objs->empty() ? nullptr : all_objs->front();
  } else {
    for (auto const obj : *all_objs) {
      if (auto const castObj{rttr::rttr_cast<T*>(obj)}) {
        return castObj;
      }
    }

    return nullptr;
  }
}


template<std::derived_from<Object> T>
auto Object::FindObjectsOfType(std::vector<T*>& out) -> std::vector<T*>& {
  auto const all_objs{sAllObjects.LockShared()};

  if constexpr (std::same_as<Object, T>) {
    out = all_objs;
  } else {
    out.clear();

    for (auto const obj : *all_objs) {
      if (auto const castObj{rttr::rttr_cast<T*>(obj)}) {
        out.emplace_back(castObj);
      }
    }
  }

  return out;
}


template<std::derived_from<Object> T>
auto Object::FindObjectsOfType() -> std::vector<T*> {
  std::vector<T*> ret;
  FindObjectsOfType<T>(ret);
  return ret;
}


template<typename... Args>
auto MakeUniqueObject(rttr::type const& type, Args&&... args) -> std::unique_ptr<Object> {
  if (type.is_derived_from(rttr::type::get<Object>())) {
    return std::unique_ptr<Object>{type.create(std::forward<Args>(args)...).template get_value<Object*>()};
  }

  return nullptr;
}
}
