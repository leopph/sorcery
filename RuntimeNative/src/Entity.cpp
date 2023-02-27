#include "Entity.hpp"

#include <format>

#include "TransformComponent.hpp"
#include "CubeModelComponent.hpp"
#include "CameraComponent.hpp"
#include "LightComponents.hpp"
#include "ManagedRuntime.hpp"
#include "Systems.hpp"

#include <mono/metadata/class.h>
#include <mono/metadata/reflection.h>
#include <mono/metadata/object.h>

#include <functional>
#include <iostream>

#include "BehaviorComponent.hpp"


namespace leopph {
namespace {
template<std::derived_from<Component> T>
std::unique_ptr<Component> instantiate() {
	return std::make_unique<T>();
}


std::unordered_map<std::string_view, std::function<std::unique_ptr<Component>()>> const gComponentInstantiators
{
	{ "CubeModel", instantiate<CubeModelComponent> },
	{ "Camera", instantiate<CameraComponent> },
	{ "Light", instantiate<LightComponent> }
};

std::vector<Entity*> gEntityCache;
}


auto Entity::FindEntityByName(std::string_view const name) -> Entity* {
	FindObjectsOfType(gEntityCache);
	for (auto* const entity : gEntityCache) {
		if (entity->GetName() == name) {
			return entity;
		}
	}
	return nullptr;
}


Entity::Entity() {
	auto constexpr defaultEntityName{ "New Entity" };
	SetName(defaultEntityName);
	FindObjectsOfType(gEntityCache);

	bool isNameUnique{ false };
	std::size_t index{ 1 };
	while (!isNameUnique) {
		isNameUnique = true;
		for (auto const entity : gEntityCache) {
			if (entity != this && entity->GetName() == GetName()) {
				SetName(std::format("{} ({})", defaultEntityName, index));
				++index;
				isNameUnique = false;
				break;
			}
		}
	}
}


auto Entity::GetScene() const -> Scene& {
	return *mScene;
}


auto Entity::GetTransform() const -> TransformComponent& {
	if (!mTransform) {
		mTransform = GetComponent<TransformComponent>();
	}
	return *mTransform;
}


auto Entity::CreateComponent(MonoClass* const componentClass) -> Component* {
	if (mono_class_is_subclass_of(componentClass, mono_class_from_name(gManagedRuntime.GetManagedImage(), "leopph", "Behavior"), false)) {
		auto behavior{ std::make_unique<BehaviorComponent>() };
		behavior->CreateAndInitManagedObject(componentClass);
		auto const ret{ behavior.get() };
		AddComponent(std::move(behavior));
		return ret;
	}

	std::string_view const className{ mono_class_get_name(componentClass) };

	if (className == "Transform") {
		return &GetTransform();
	}

	if (auto const it{ gComponentInstantiators.find(className) }; it != std::end(gComponentInstantiators)) {
		auto component{ it->second() };
		component->CreateManagedObject();
		auto const ret{ component.get() };
		AddComponent(std::move(component));
		return ret;
	}

	return nullptr;
}


auto Entity::AddComponent(std::unique_ptr<Component> component) -> void {
	if (component) {
		component->SetEntity(this);
		mComponents.push_back(std::move(component));
	}
}


auto Entity::DestroyComponent(Component* const component) -> void {
	if (component) {
		if (component->GetEntity()->GetGuid() != GetGuid() || component->GetEntity()->GetTransform().GetGuid() == component->GetGuid()) {
			return;
		}
		std::erase_if(mComponents, [component](auto const& attachedComponent) {
			return attachedComponent->GetGuid() == component->GetGuid();
		});
	}
}

auto Entity::CreateManagedObject() -> void {
	return ManagedAccessObject::CreateManagedObject("leopph", "Entity");
}

auto ObjectInstantiatorFor<Entity>::Instantiate() -> Object* {
	return nullptr; // TODO
}

Object::Type const Entity::SerializationType{ Type::Entity };

auto Entity::SetScene(Scene* const scene) -> void {
	mScene = scene;
}

auto Entity::New() -> Entity* {
	return nullptr; // TODO
}

auto Entity::NewForManagedObject(MonoObject* const managedObject) -> Entity* {
	auto const entity{ New() };
	entity->AcquireManagedObject(managedObject);
	return entity;
}


auto Entity::GetSerializationType() const -> Type {
	return Type::Entity;
}


auto Entity::SerializeTextual(YAML::Node& node) const -> void {
	node["name"] = GetName().data();
	for (auto const& component : mComponents) {
		node["components"].push_back(component->GetGuid().ToString());
	}
}


auto Entity::DeserializeTextual(YAML::Node const& node) -> void {
	if (!node["name"].IsScalar()) {
		std::cerr << "Failed to deserialize name of Entity " << GetGuid().ToString() << ". Invalid data." << std::endl;
	}
	else {
		SetName(node["name"].as<std::string>());
	}

	for (auto it{ node["components"].begin() }; it != node["components"].end(); ++it) {
		if (!it->IsScalar()) {
			std::cerr << "Failed to deserialize a Component of Entity " << GetGuid().ToString() << ". Invalid data." << std::endl;
		}
		else {
			auto const guidStr{ it->as<std::string>() };
			auto const component{ (dynamic_cast<Component*>(Object::FindObjectByGuid(Guid::Parse(guidStr)))) };
			if (!component) {
				std::cerr << "Failed to deserialize a Component of Entity " << GetGuid().ToString() << ". Guid " << guidStr << " does not belong to any Component." << std::endl;
			}
			else {
				AddComponent(std::unique_ptr<Component>{ component });
			}
		}
	}
}


namespace managedbindings {
void CreateNativeEntity(MonoObject* managedEntity) {
	auto const entity{ Entity::NewForManagedObject(managedEntity) };

	auto transform = std::make_unique<TransformComponent>();
	transform->CreateManagedObject();

	entity->AddComponent(std::move(transform));
}


MonoObject* GetEntityTransform(MonoObject* managedEntity) {
	auto const nativeEntity = ManagedAccessObject::GetNativePtrFromManagedObjectAs<Entity*>(managedEntity);
	return nativeEntity->GetTransform().GetManagedObject();
}


MonoString* GetEntityName(MonoObject* managedEntity) {
	return mono_string_new_wrapper(ManagedAccessObject::GetNativePtrFromManagedObjectAs<Entity*>(managedEntity)->GetName().data());
}


void SetEntityName(MonoObject* managedEntity, MonoString* managedName) {
	ManagedAccessObject::GetNativePtrFromManagedObjectAs<Entity*>(managedEntity)->SetName(mono_string_to_utf8(managedName));
}


MonoObject* EntityCreateComponent(Entity* const entity, MonoReflectionType* const componentType) {
	auto const componentClass{ mono_type_get_class(mono_reflection_type_get_type(componentType)) };
	auto const component{ entity->CreateComponent(componentClass) };
	return component ? component->GetManagedObject() : nullptr;
}
}
}
