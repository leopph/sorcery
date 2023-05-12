#pragma once

#include <Object.hpp>
#include <BehaviorComponent.hpp>
#include <CameraComponent.hpp>
#include <StaticMeshComponent.hpp>
#include <Entity.hpp>
#include <LightComponents.hpp>
#include <Material.hpp>
#include <TransformComponent.hpp>
#include "../Importer.hpp"
#include "Texture2D.hpp"
#include "Cubemap.hpp"
#include "SceneElement.hpp"
#include "SkyboxComponent.hpp"

#include <concepts>
#include <filesystem>
#include <stdexcept>


namespace leopph::editor {
class ObjectWrapperManager;
class Context;


class ObjectWrapper : public ObjectInstantiator {
public:
  virtual auto OnDrawProperties(Context& context, Object& object) -> void = 0;
  [[nodiscard]] virtual auto GetImporter() -> Importer& = 0;
  virtual auto OnDrawGizmosSelected(Context& context, Object& object) -> void = 0;
};


template<typename Wrapped>
class ObjectWrapperFor : public ObjectWrapper {
public:
  auto OnDrawProperties([[maybe_unused]] Context& context, [[maybe_unused]] Object& object) -> void override {}


  [[nodiscard]] auto Instantiate() -> Object* override {
    if constexpr (std::derived_from<Wrapped, SceneElement>) {
      return new Wrapped{};
    } else {
      throw std::runtime_error{ "The wrapped type has no associated instantiation function." };
    }
  }


  [[nodiscard]] auto GetImporter() -> Importer& override {
    throw std::runtime_error{ "The wrapped type has no associated importer." };
  }


  auto OnDrawGizmosSelected([[maybe_unused]] Context& context, [[maybe_unused]] Object& object) -> void override {}
};


template<>
auto ObjectWrapperFor<BehaviorComponent>::OnDrawProperties(Context& context, Object& object) -> void;

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
auto ObjectWrapperFor<Mesh>::GetImporter() -> Importer&;

template<>
auto ObjectWrapperFor<Texture2D>::GetImporter() -> Importer&;

template<>
auto ObjectWrapperFor<Material>::GetImporter() -> Importer&;

template<>
auto ObjectWrapperFor<Scene>::GetImporter() -> Importer&;

template<>
auto ObjectWrapperFor<Cubemap>::GetImporter() -> Importer&;

template<>
auto ObjectWrapperFor<Entity>::OnDrawGizmosSelected(Context& context, Object& object) -> void;

template<>
auto ObjectWrapperFor<LightComponent>::OnDrawGizmosSelected(Context& context, Object& object) -> void;
}
