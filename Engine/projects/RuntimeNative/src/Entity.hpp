#pragma once

#include "ManagedAccessObject.hpp"
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


	class Entity : public ManagedAccessObject {
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

		LEOPPHAPI [[nodiscard]] Transform& GetTransform();

		LEOPPHAPI Component* CreateComponent(MonoClass* componentClass);

		template<std::derived_from<Component> T>
		T* CreateComponent() {
			if constexpr (std::is_same_v<T, Transform>) {
				if (mTransform) {
					return mTransform;
				}
			}

			auto const component = new T{ this };

			if constexpr (std::is_same_v<T, Transform>) {
				mTransform = component;
			}

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

		void DeleteComponent(Component* component);
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