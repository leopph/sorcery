#pragma once

#include <Object.hpp>
#include <BehaviorComponent.hpp>
#include <CameraComponent.hpp>
#include <ModelComponent.hpp>
#include <Entity.hpp>
#include <LightComponents.hpp>
#include <Material.hpp>
#include <TransformComponent.hpp>
#include "Importer.hpp"
#include "Texture2D.hpp"
#include "Cubemap.hpp"
#include "SceneElement.hpp"

#include <concepts>
#include <filesystem>
#include <stdexcept>


namespace leopph::editor {
class EditorObjectFactoryManager;
class Context;

class EditorObjectWrapper : public ObjectInstantiator {
public:
	virtual auto OnGui(Context& context, Object& object) -> void = 0;
	[[nodiscard]] virtual auto GetImporter() -> Importer& = 0;
};

template<typename Wrapped>
class EditorObjectWrapperFor : public EditorObjectWrapper {
public:
	auto OnGui([[maybe_unused]] Context& context, [[maybe_unused]] Object& object) -> void override {}

	[[nodiscard]] auto Instantiate() -> Object* override {
		if constexpr (std::derived_from<Wrapped, SceneElement>) {
			return new Wrapped{};
		}
		else {
			throw std::runtime_error{ "The wrapped type has no associated instantiation function." };
		}
	}

	[[nodiscard]] auto GetImporter() -> Importer& override {
		throw std::runtime_error{ "The wrapped type has no associated importer." };
	}
};

template<>
auto EditorObjectWrapperFor<BehaviorComponent>::OnGui(Context& context, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<CameraComponent>::OnGui(Context& context, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<ModelComponent>::OnGui(Context& context, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<Entity>::OnGui(Context& context, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<LightComponent>::OnGui(Context& context, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<Material>::OnGui(Context& context, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<TransformComponent>::OnGui(Context& context, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<Mesh>::OnGui(Context& context, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<Texture2D>::OnGui(Context& context, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<Scene>::OnGui(Context& context, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<Entity>::Instantiate() -> Object*;

template<>
auto EditorObjectWrapperFor<Mesh>::GetImporter() -> Importer&;

template<>
auto EditorObjectWrapperFor<Texture2D>::GetImporter() -> Importer&;

template<>
auto EditorObjectWrapperFor<Material>::GetImporter() -> Importer&;

template<>
auto EditorObjectWrapperFor<Scene>::GetImporter() -> Importer&;

template<>
auto EditorObjectWrapperFor<Cubemap>::GetImporter() -> Importer&;
}
