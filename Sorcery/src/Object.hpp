#pragma once

#include "Core.hpp"
#include "Util.hpp"
#include "Reflection.hpp"

#include <concepts>
#include <string>
#include <vector>


namespace sorcery {
class Object {
  RTTR_ENABLE()
  RTTR_REGISTRATION_FRIEND
  LEOPPHAPI static std::vector<ObserverPtr<Object>> sAllObjects;

  std::string mName{ "New Object" };

public:
  LEOPPHAPI Object();
  Object(Object const& other) = delete;
  Object(Object&& other) = delete;

  LEOPPHAPI virtual ~Object();

  auto operator=(Object const& other) -> void = delete;
  auto operator=(Object&& other) -> void = delete;

  [[nodiscard]] LEOPPHAPI auto GetName() const noexcept -> std::string const&;
  LEOPPHAPI auto SetName(std::string const& name) -> void;

  virtual auto OnDrawProperties() -> void {}
  virtual auto OnDrawGizmosSelected() -> void {}

  template<std::derived_from<Object> T>
  [[nodiscard]] static auto FindObjectOfType() -> T*;

  template<std::derived_from<Object> T>
  static auto FindObjectsOfType(std::vector<T*>& out) -> std::vector<T*>&;

  template<std::derived_from<Object> T>
  [[nodiscard]] auto FindObjectsOfType() -> std::vector<T*>;

  template<std::derived_from<Object> T>
  static auto FindObjectsOfType(std::vector<Object*>& out) -> std::vector<Object*>&;

  LEOPPHAPI static auto DestroyAll() -> void;
};


template<std::derived_from<Object> T>
auto Object::FindObjectOfType() -> T* {
  for (auto const obj : sAllObjects) {
    if (auto const castObj{ dynamic_cast<T*>(obj) }; castObj) {
      return castObj;
    }
  }
  return nullptr;
}


template<std::derived_from<Object> T>
auto Object::FindObjectsOfType(std::vector<T*>& out) -> std::vector<T*>& {
  out.clear();
  for (auto const obj : sAllObjects) {
    if (auto const castObj{ dynamic_cast<T*>(obj) }; castObj) {
      out.emplace_back(castObj);
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
auto Object::FindObjectsOfType(std::vector<Object*>& out) -> std::vector<Object*>& {
  out.clear();
  for (auto const obj : sAllObjects) {
    if (dynamic_cast<T*>(obj)) {
      out.emplace_back(obj);
    }
  }
  return out;
}
}
