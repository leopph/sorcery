#pragma once

#include <memory>
#include <string>
#include <string_view>

#include <sorcery/Core.hpp>
#include <sorcery/object_id.hpp>
#include <sorcery/reflection.hpp>


namespace sorcery {
class Object {
  RTTR_ENABLE()
  RTTR_REGISTRATION_FRIEND

protected:
  SORCERYAPI Object();
  SORCERYAPI Object(Object const& other);
  Object(Object&& other) = delete;

public:
  SORCERYAPI virtual ~Object();

  auto operator=(Object const& other) -> void = delete;
  auto operator=(Object&& other) -> void = delete;

  [[nodiscard]] SORCERYAPI
  auto GetName() const noexcept -> std::string_view;
  SORCERYAPI
  auto SetName(std::string_view name) -> void;

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


#include <sorcery/object.inl>
