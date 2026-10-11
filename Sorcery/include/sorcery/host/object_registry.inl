#pragma once

namespace sorcery {
template<std::derived_from<Object> T>
auto ObjectRegistry::FindObjectOfType() -> ObjectPtr<T> {
  auto const data{data_.LockShared()};

  for (auto const& [obj, gen] : data->slots) {
    if (obj) {
      if constexpr (std::same_as<Object, T>) {
        return MakeObjectPtr(obj);
      } else {
        if (auto const typed = ReflCast<T>(obj)) {
          return MakeObjectPtr(typed);
        }
      }
    }
  }

  return nullptr;
}


template<std::derived_from<Object> T>
auto ObjectRegistry::FindObjectsOfType(std::vector<ObjectPtr<T>>& out) -> std::vector<ObjectPtr<T>>& {
  out.clear();

  auto const data{data_.LockShared()};

  for (auto const& [obj, gen] : data->slots) {
    if (obj) {
      if constexpr (std::same_as<Object, T>) {
        out.emplace_back(obj);
      } else {
        if (auto const typed = ReflCast<T>(obj)) {
          out.emplace_back(typed);
        }
      }
    }
  }

  return out;
}


template<std::derived_from<Object> T>
auto ObjectRegistry::FindObjectsOfType() -> std::vector<ObjectPtr<T>> {
  std::vector<T*> ret;
  FindObjectsOfType<T>(ret);
  return ret;
}
}
