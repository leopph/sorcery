#pragma once

#include "SceneElement.hpp"

#include <string>
#include <string_view>
#include <memory>
#include <span>
#include <vector>
#include <concepts>

using MonoString = struct _MonoString;
using MonoReflectionType = struct _MonoReflectionType;


namespace leopph {
	class Component;
	class Transform;
	class Scene;


	class Entity : public SceneElement {
		friend class Scene;

	private:
		static std::vector<Entity*> sAllEntities;

		std::string mName{ "Entity" };
		Scene* mScene{ nullptr };
		mutable Transform* mTransform{ nullptr };
		std::vector<std::unique_ptr<Component>> mComponents;

		Entity();

		auto SetScene(Scene* scene) -> void;

	public:
		[[nodiscard]] LEOPPHAPI static auto GetAllEntities()->std::span<Entity* const>;
		LEOPPHAPI static auto GetAllEntities(std::vector<Entity*>& outEntities) -> std::vector<Entity*>&;
		[[nodiscard]] LEOPPHAPI static auto FindEntityByName(std::string_view name) -> Entity*;

		LEOPPHAPI ~Entity();

		[[nodiscard]] LEOPPHAPI auto GetSerializationType() const->Type override;
		LEOPPHAPI auto Serialize(YAML::Node& node) const -> void override;
		LEOPPHAPI auto Deserialize(YAML::Node const& node) -> void override;

		[[nodiscard]] LEOPPHAPI auto GetName() const -> std::string_view;
		LEOPPHAPI auto SetName(std::string name) -> void;
		[[nodiscard]] LEOPPHAPI auto GetScene() const -> Scene&;
		[[nodiscard]] LEOPPHAPI auto GetTransform() const -> Transform&;

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
	};


	namespace managedbindings {
		void CreateNativeEntity(MonoObject* managedEntity);
		MonoObject* GetEntityTransform(MonoObject* managedEntity);
		MonoString* GetEntityName(MonoObject* managedEntity);
		void SetEntityName(MonoObject* managedEntity, MonoString* managedName);
		MonoObject* EntityCreateComponent(Entity* entity, MonoReflectionType* componentType);
	}
}