#pragma once

#include "Core.hpp"
#include "Reflection.hpp"

#include <concepts>
#include <mutex>
#include <string>
#include <vector>


namespace sorcery {
class Object {
  RTTR_ENABLE()
  RTTR_REGISTRATION_FRIEND

protected:
  Object() = default;
  Object(Object const& other) = default;
  Object(Object&& other) noexcept = default;

public:
  virtual ~Object() = default;

  auto operator=(Object const& other) -> void = delete;
  auto operator=(Object&& other) -> void = delete;

  [[nodiscard]] LEOPPHAPI auto GetName() const noexcept -> std::string const&;
  LEOPPHAPI auto SetName(std::string const& name) -> void;

  LEOPPHAPI virtual auto OnInit() -> void;
  LEOPPHAPI virtual auto OnDestroy() -> void;
  LEOPPHAPI virtual auto OnDrawProperties(bool& changed) -> void;
  virtual auto OnDrawGizmosSelected() -> void {}

  template<std::derived_from<Object> T>
  [[nodiscard]] static auto FindObjectOfType() -> T*;

  template<std::derived_from<Object> T>
  static auto FindObjectsOfType(std::vector<T*>& out) -> std::vector<T*>&;

  template<std::derived_from<Object> T>
  [[nodiscard]] auto FindObjectsOfType() -> std::vector<T*>;

  LEOPPHAPI static auto DestroyAll() -> void;

private:
  LEOPPHAPI static std::vector<Object*> sAllObjects;
  LEOPPHAPI static std::recursive_mutex sAllObjectsMutex;

  std::string mName{"New Object"};
};


template<std::derived_from<Object> T>
[[nodiscard]] auto CreateAndInitialize() -> T*;

LEOPPHAPI auto Destroy(Object& obj) -> void;


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


template<std::derived_from<Object> T>
auto CreateAndInitialize() -> T* {
  auto const obj{new T{}};
  obj->OnInit();
  return obj;
}
}
