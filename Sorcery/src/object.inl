#pragma once

#include <utility>


namespace sorcery {
template<std::derived_from<Object> T>
auto Object::FindObjectOfType() -> T* {
  std::unique_lock const lock{sAllObjectsMutex};

  if constexpr (std::same_as<Object, T>) {
    return sAllObjects.empty() ? nullptr : sAllObjects.front();
  } else {
    for (auto const obj : sAllObjects) {
      if (auto const castObj{rttr::rttr_cast<T*>(obj)}) {
        return castObj;
      }
    }

    return nullptr;
  }
}


template<std::derived_from<Object> T>
auto Object::FindObjectsOfType(std::vector<T*>& out) -> std::vector<T*>& {
  std::unique_lock const lock{sAllObjectsMutex};

  if constexpr (std::same_as<Object, T>) {
    out = sAllObjects;
  } else {
    out.clear();

    for (auto const obj : sAllObjects) {
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


template<std::derived_from<Object> ObjectType, typename... Args>
auto Create(Args&&... args) -> std::unique_ptr<ObjectType> {
  return std::make_unique<ObjectType>(std::forward<Args>(args)...);
}


template<std::derived_from<Object> ObjectType, typename... Args>
auto CreateInit(Args&&... args) -> std::unique_ptr<ObjectType> {
  auto obj{Create<ObjectType>(std::forward<Args>(args)...)};
  obj->Initialize();
  return obj;
}
}
