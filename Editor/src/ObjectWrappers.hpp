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
class ObjectFactoryManager;

class BasicObjectWrapper {
public:
	virtual ~BasicObjectWrapper() = default;
	virtual auto OnGui(ObjectFactoryManager const& objectFactoryManager, Object& object) -> void = 0;
};

template<typename Wrapped>
class ObjectWrapper : public BasicObjectWrapper {
public:
	auto OnGui([[maybe_unused]] ObjectFactoryManager const& objectFactoryManager, [[maybe_unused]] Object& object) -> void override {}
};

template<>
class ObjectWrapper<BehaviorComponent> : public BasicObjectWrapper {
public:
	auto OnGui(ObjectFactoryManager const& objectFactoryManager, Object& object) -> void override;
};

template<>
class ObjectWrapper<CameraComponent> : public BasicObjectWrapper {
public:
	auto OnGui(ObjectFactoryManager const& objectFactoryManager, Object& object) -> void override;
};

template<>
class ObjectWrapper<CubeModelComponent> : public BasicObjectWrapper {
public:
	auto OnGui(ObjectFactoryManager const& objectFactoryManager, Object& object) -> void override;
};

template<>
class ObjectWrapper<Entity> : public BasicObjectWrapper {
public:
	auto OnGui(ObjectFactoryManager const& objectFactoryManager, Object& object) -> void override;
};

template<>
class ObjectWrapper<LightComponent> : public BasicObjectWrapper {
public:
	auto OnGui(ObjectFactoryManager const& objectFactoryManager, Object& object) -> void override;
};

template<>
class ObjectWrapper<Material> : public BasicObjectWrapper {
public:
	auto OnGui(ObjectFactoryManager const& objectFactoryManager, Object& object) -> void override;
};

template<>
class ObjectWrapper<TransformComponent> : public BasicObjectWrapper {
public:
	auto OnGui(ObjectFactoryManager const& objectFactoryManager, Object& object) -> void override;
};
}
