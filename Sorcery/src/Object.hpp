#pragma once

#include "Core.hpp"
#include "Reflection.hpp"

#include <concepts>
#include <memory>
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
  LEOPPHAPI virtual ~Object();

  auto operator=(Object const& other) -> void = delete;
  auto operator=(Object&& other) -> void = delete;

  [[nodiscard]] LEOPPHAPI auto GetName() const noexcept -> std::string const&;
  LEOPPHAPI auto SetName(std::string const& name) -> void;

  LEOPPHAPI virtual auto Initialize() -> void;
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


template<std::derived_from<Object> ObjectType, typename... Args>
[[nodiscard]] auto Create(Args&&... args) -> std::unique_ptr<ObjectType>;

template<std::derived_from<Object> ObjectType, typename... Args>
[[nodiscard]] auto CreateInit(Args&&... args) -> std::unique_ptr<ObjectType>;
}


#include "object.inl"
