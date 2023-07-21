#pragma once

#include <Object.hpp>
#include <CameraComponent.hpp>
#include <StaticMeshComponent.hpp>
#include <Entity.hpp>
#include <LightComponents.hpp>
#include <Material.hpp>
#include <TransformComponent.hpp>
#include "Texture2D.hpp"
#include "Cubemap.hpp"
#include "SceneObject.hpp"
#include "SkyboxComponent.hpp"
#include "RigidBodyComponent.hpp"

#include <concepts>
#include <filesystem>
#include <stdexcept>


namespace sorcery::mage {
class ObjectWrapperManager;
class Context;


class ObjectWrapper : public ObjectInstantiator {
public:
  virtual auto OnDrawProperties(Context& context, Object& object) -> void = 0;
  virtual auto OnDrawGizmosSelected(Context& context, Object& object) -> void = 0;
  [[nodiscard]] virtual auto GetWrappedType() const noexcept -> Object::Type = 0;
};


template<typename Wrapped>
class ObjectWrapperFor : public ObjectWrapper {
public:
  auto OnDrawProperties([[maybe_unused]] Context& context, [[maybe_unused]] Object& object) -> void override {}


  [[nodiscard]] auto Instantiate() -> Object* override {
    if constexpr (std::derived_from<Wrapped, SceneObject>) {
      return new Wrapped{};
    } else {
      throw std::runtime_error{ "The wrapped type has no associated instantiation function." };
    }
  }


  auto OnDrawGizmosSelected([[maybe_unused]] Context& context, [[maybe_unused]] Object& object) -> void override {}


  [[nodiscard]] auto GetWrappedType() const noexcept -> Object::Type override {
    return Wrapped::SerializationType;
  }
};


template<>
auto ObjectWrapperFor<CameraComponent>::OnDrawProperties(Context& context, Object& object) -> void;

template<>
auto ObjectWrapperFor<StaticMeshComponent>::OnDrawProperties(Context& context, Object& object) -> void;

template<>
auto ObjectWrapperFor<Entity>::OnDrawProperties(Context& context, Object& object) -> void;

template<>
auto ObjectWrapperFor<LightComponent>::OnDrawProperties(Context& context, Object& object) -> void;

template<>
auto ObjectWrapperFor<Material>::OnDrawProperties(Context& context, Object& object) -> void;

template<>
auto ObjectWrapperFor<TransformComponent>::OnDrawProperties(Context& context, Object& object) -> void;

template<>
auto ObjectWrapperFor<Mesh>::OnDrawProperties(Context& context, Object& object) -> void;

template<>
auto ObjectWrapperFor<Texture2D>::OnDrawProperties(Context& context, Object& object) -> void;

template<>
auto ObjectWrapperFor<Scene>::OnDrawProperties(Context& context, Object& object) -> void;

template<>
auto ObjectWrapperFor<SkyboxComponent>::OnDrawProperties(Context& context, Object& object) -> void;

template<>
auto ObjectWrapperFor<Entity>::Instantiate() -> Object*;

template<>
auto ObjectWrapperFor<Entity>::OnDrawGizmosSelected(Context& context, Object& object) -> void;

template<>
auto ObjectWrapperFor<LightComponent>::OnDrawGizmosSelected(Context& context, Object& object) -> void;
}
