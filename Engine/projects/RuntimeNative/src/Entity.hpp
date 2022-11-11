#pragma once

#include "SceneElement.hpp"
#include "Math.hpp"

#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>
#include <span>
#include <vector>
#include <optional>
#include <concepts>
#include <type_traits>

using MonoString = struct _MonoString;
using MonoReflectionType = struct _MonoReflectionType;


namespace leopph {
	class Component;
	class Transform;
	class Scene;


	class Entity : public SceneElement {
	private:
		Entity() = default;

		std::vector<std::unique_ptr<Component>> mComponents;
		Transform* mTransform{ nullptr };

	public:
		std::string name;
		Scene* scene{ nullptr };

		Entity(Entity const& other) = delete;
		Entity& operator=(Entity const& other) = delete;

		// Creates a raw Entity without a Transform or a managed object.
		// Registers itself in the active scene.
		LEOPPHAPI static Entity* Create();

		[[nodiscard]] LEOPPHAPI Transform& GetTransform();

		[[nodiscard]] LEOPPHAPI auto GetSerializationType() const->Type override;
		LEOPPHAPI auto Serialize(YAML::Node& node) const -> void override;
		LEOPPHAPI auto Deserialize(YAML::Node const& node) -> void override;
		LEOPPHAPI auto DeserializeResolveReferences(YAML::Node const& node) -> void override;

		LEOPPHAPI Component* CreateComponent(MonoClass* componentClass);

		template<std::derived_from<Component> T>
		T* CreateComponent() {
			auto const component = new T{};
			component->entity = this;
			mComponents.emplace_back(component);
			return component;
		}

		template<std::derived_from<Component> T>
		T* GetComponent() const {
			for (auto const& component : mComponents) {
				if (auto const castPtr = dynamic_cast<T*>(component.get())) {
					return castPtr;
				}
			}

			return nullptr;
		}

		template<std::derived_from<Component> T>
		std::vector<T*>& GetComponents(std::vector<T*>& outComponents) const {
			outComponents.clear();

			for (auto const& component : mComponents) {
				if (auto const castPtr = dynamic_cast<T*>(component.get())) {
					outComponents.emplace_back(castPtr);
				}
			}

			return outComponents;
		}

		LEOPPHAPI void DeleteComponent(Component* component);
	};

	static_assert(!std::is_copy_constructible_v<Entity>);
	static_assert(!std::is_copy_assignable_v<Entity>);
	static_assert(!std::is_move_constructible_v<Entity>);
	static_assert(!std::is_move_assignable_v<Entity>);


	LEOPPHAPI std::vector<Entity*>& GetEntities(std::vector<Entity*>& outEntities);
	LEOPPHAPI void DeleteEntity(Entity* entity);

	Entity* FindEntity(std::string_view name);


	namespace managedbindings {
		void CreateNativeEntity(MonoObject* managedEntity);
		MonoObject* GetEntityTransform(MonoObject* managedEntity);
		MonoString* GetEntityName(MonoObject* managedEntity);
		void SetEntityName(MonoObject* managedEntity, MonoString* managedName);
		MonoObject* EntityCreateComponent(Entity* entity, MonoReflectionType* componentType);
	}
}