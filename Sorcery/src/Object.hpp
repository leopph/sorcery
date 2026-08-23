#pragma once

#include <memory>
#include <string>

#include "Core.hpp"
#include "object_id.hpp"
#include "reflection.hpp"


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

private:
  std::string name_{"New Object"};
  ObjectId id_;
};


template<typename... Args>
[[nodiscard]] auto MakeUniqueObject(rttr::type const& type, Args&&... args) -> std::unique_ptr<Object>;
}


#include "object.inl"
