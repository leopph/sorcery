#pragma once

#include "Core.hpp"
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
  [[nodiscard]] static auto FindObjectOfType() -> ObserverPtr<T>;

  template<std::derived_from<Object> T>
  static auto FindObjectsOfType(std::vector<ObserverPtr<T>>& out) -> std::vector<ObserverPtr<T>>&;

  template<std::derived_from<Object> T>
  [[nodiscard]] auto FindObjectsOfType() -> std::vector<ObserverPtr<T>>;

  LEOPPHAPI static auto DestroyAll() -> void;
};


template<std::derived_from<Object> T>
auto Object::FindObjectOfType() -> ObserverPtr<T> {
  if constexpr (std::same_as<Object, T>) {
    return sAllObjects.empty()
             ? nullptr
             : sAllObjects.front();
  } else {
    for (auto const obj : sAllObjects) {
      if (auto const castObj{ rttr::rttr_cast<ObserverPtr<T>>(obj) }) {
        return castObj;
      }
    }

    return nullptr;
  }
}


template<std::derived_from<Object> T>
auto Object::FindObjectsOfType(std::vector<ObserverPtr<T>>& out) -> std::vector<ObserverPtr<T>>& {
  if constexpr (std::same_as<Object, T>) {
    out = sAllObjects;
  } else {
    out.clear();

    for (auto const obj : sAllObjects) {
      if (auto const castObj{ rttr::rttr_cast<ObserverPtr<T>>(obj) }) {
        out.emplace_back(castObj);
      }
    }
  }

  return out;
}


template<std::derived_from<Object> T>
auto Object::FindObjectsOfType() -> std::vector<ObserverPtr<T>> {
  std::vector<ObserverPtr<T>> ret;
  FindObjectsOfType<T>(ret);
  return ret;
}
}
