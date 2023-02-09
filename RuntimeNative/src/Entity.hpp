#pragma once

#include "ManagedAccessObject.hpp"
#include "Component.hpp"

#include <string>
#include <string_view>
#include <memory>
#include <span>
#include <vector>
#include <concepts>

using MonoString = struct _MonoString;
using MonoReflectionType = struct _MonoReflectionType;


namespace leopph {
class Scene;

class Entity final : public ManagedAccessObject {
	friend class Scene;

	Scene* mScene{ nullptr };
	mutable TransformComponent* mTransform{ nullptr };
	std::vector<std::unique_ptr<Component>> mComponents;

	Entity();

	auto SetScene(Scene* scene) -> void;

public:
	LEOPPHAPI static auto New() -> Entity*;
	LEOPPHAPI static auto NewForManagedObject(MonoObject* managedObject) -> Entity*;

	[[nodiscard]] LEOPPHAPI static auto FindEntityByName(std::string_view name) -> Entity*;

	[[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;
	LEOPPHAPI static Object::Type const SerializationType;

	LEOPPHAPI auto SerializeTextual(YAML::Node& node) const -> void override;
	LEOPPHAPI auto DeserializeTextual(YAML::Node const& node) -> void override;

	[[nodiscard]] LEOPPHAPI auto GetScene() const -> Scene&;
	[[nodiscard]] LEOPPHAPI auto GetTransform() const -> TransformComponent&;

	LEOPPHAPI auto CreateComponent(MonoClass* componentClass) -> Component*;
	LEOPPHAPI auto AddComponent(std::unique_ptr<Component> component) -> void;
	LEOPPHAPI auto DestroyComponent(Component* component) -> void;

	template<std::derived_from<Component> T>
	auto GetComponent() const -> T* {
		for (auto const& component : mComponents) {
			if (auto const castPtr = dynamic_cast<T*>(component.get())) {
				return castPtr;
			}
		}
		return nullptr;
	}

	template<std::derived_from<Component> T>
	auto GetComponents(std::vector<T*>& outComponents) const -> std::vector<T*>& {
		outComponents.clear();
		for (auto const& component : mComponents) {
			if (auto const castPtr = dynamic_cast<T*>(component.get())) {
				outComponents.emplace_back(castPtr);
			}
		}
		return outComponents;
	}

	LEOPPHAPI auto OnGui() -> void override;

	LEOPPHAPI auto CreateManagedObject() -> void override;
};


template<>
class ObjectInstantiatorFor<Entity> : public ObjectInstantiator {
public:
	[[nodiscard]] LEOPPHAPI auto Instantiate() -> Object* override;
};


namespace managedbindings {
void CreateNativeEntity(MonoObject* managedEntity);
MonoObject* GetEntityTransform(MonoObject* managedEntity);
MonoString* GetEntityName(MonoObject* managedEntity);
void SetEntityName(MonoObject* managedEntity, MonoString* managedName);
MonoObject* EntityCreateComponent(Entity* entity, MonoReflectionType* componentType);
}
}
