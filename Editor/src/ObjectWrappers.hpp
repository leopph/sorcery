#pragma once

#include <Object.hpp>
#include <BehaviorComponent.hpp>
#include <CameraComponent.hpp>
#include <CubeModelComponent.hpp>
#include <Entity.hpp>
#include <LightComponents.hpp>
#include <Material.hpp>
#include <TransformComponent.hpp>


namespace leopph::editor {
class EditorObjectFactoryManager;

class EditorObjectWrapper : public ObjectInstantiator {
public:
	virtual auto OnGui(EditorObjectFactoryManager const& objectFactoryManager, Object& object) -> void = 0;
};

template<typename Wrapped>
class EditorObjectWrapperFor : public EditorObjectWrapper {
public:
	auto OnGui([[maybe_unused]] EditorObjectFactoryManager const& objectFactoryManager, [[maybe_unused]] Object& object) -> void override {}
	[[nodiscard]] auto Instantiate() -> Object* override;
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
auto EditorObjectWrapperFor<CubeModelComponent>::OnGui(EditorObjectFactoryManager const& objectFactoryManager, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<Entity>::OnGui(EditorObjectFactoryManager const& objectFactoryManager, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<LightComponent>::OnGui(EditorObjectFactoryManager const& objectFactoryManager, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<Material>::OnGui(EditorObjectFactoryManager const& objectFactoryManager, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<TransformComponent>::OnGui(EditorObjectFactoryManager const& objectFactoryManager, Object& object) -> void;

template<>
auto EditorObjectWrapperFor<Entity>::Instantiate() -> Object*;
}
