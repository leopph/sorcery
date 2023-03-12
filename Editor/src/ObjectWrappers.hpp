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

#include <filesystem>
#include <stdexcept>


namespace leopph::editor {
class EditorObjectFactoryManager;

class EditorObjectWrapper : public ObjectInstantiator {
public:
	virtual auto OnGui(EditorObjectFactoryManager const& objectFactoryManager, Object& object) -> void = 0;
	[[nodiscard]] virtual auto GetImporter() -> Importer& = 0;
};

template<typename Wrapped>
class EditorObjectWrapperFor : public EditorObjectWrapper {
public:
	auto OnGui([[maybe_unused]] EditorObjectFactoryManager const& objectFactoryManager, [[maybe_unused]] Object& object) -> void override {}
	[[nodiscard]] auto Instantiate() -> Object* override;

	[[nodiscard]] auto GetImporter() -> Importer& override {
		throw std::runtime_error{ "The wrapped type has no associated importer." };
	}
};

template<typename Wrapped>
auto EditorObjectWrapperFor<Wrapped>::Instantiate() -> Object* {
	return new Wrapped{};
}

template<>
auto EditorObjectWrapperFor<BehaviorComponent>::OnGui(EditorObjectFactoryManager const& objectFactoryManager, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<CameraComponent>::OnGui(EditorObjectFactoryManager const& objectFactoryManager, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<ModelComponent>::OnGui(EditorObjectFactoryManager const& objectFactoryManager, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<Entity>::OnGui(EditorObjectFactoryManager const& objectFactoryManager, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<LightComponent>::OnGui(EditorObjectFactoryManager const& objectFactoryManager, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<Material>::OnGui(EditorObjectFactoryManager const& objectFactoryManager, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<TransformComponent>::OnGui(EditorObjectFactoryManager const& objectFactoryManager, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<Mesh>::OnGui(EditorObjectFactoryManager const& objectFactoryManager, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<Texture2D>::OnGui(EditorObjectFactoryManager const& objectFactoryManager, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<Entity>::Instantiate() -> Object*;

template<>
auto EditorObjectWrapperFor<Mesh>::GetImporter() -> Importer&;

template<>
auto EditorObjectWrapperFor<Texture2D>::GetImporter() -> Importer&;

template<>
auto EditorObjectWrapperFor<Material>::GetImporter() -> Importer&;
}
