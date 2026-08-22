#pragma once

#include <concepts>
#include <memory>
#include <string>
#include <vector>

#include "Core.hpp"
#include "mutex.hpp"
#include "object_id.hpp"
#include "Reflection.hpp"


namespace sorcery {
class Object {
  RTTR_ENABLE()
  RTTR_REGISTRATION_FRIEND

protected:
  SORCERYAPI Object();
  Object(Object const& other) = default;
  Object(Object&& other) noexcept = default;

public:
  SORCERYAPI virtual ~Object();

  auto operator=(Object const& other) -> void = delete;
  auto operator=(Object&& other) -> void = delete;

  [[nodiscard]] SORCERYAPI
  auto GetName() const noexcept -> std::string const&;
  SORCERYAPI
  auto SetName(std::string const& name) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetId() const -> ObjectId const&;

  virtual auto OnDrawGizmosSelected() -> void {}

  template<std::derived_from<Object> T>
  [[nodiscard]] static auto FindObjectOfType() -> T*;

  template<std::derived_from<Object> T>
  static auto FindObjectsOfType(std::vector<T*>& out) -> std::vector<T*>&;

  template<std::derived_from<Object> T>
  [[nodiscard]] auto FindObjectsOfType() -> std::vector<T*>;

private:
  std::string name_{"New Object"};
  ObjectId id_;

  SORCERYAPI static Mutex<std::vector<Object*>, true> sAllObjects;
};


template<typename... Args>
[[nodiscard]] auto MakeUniqueObject(rttr::type const& type, Args&&... args) -> std::unique_ptr<Object>;
}


#include "object.inl"
