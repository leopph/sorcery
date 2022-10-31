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

using MonoString = struct _MonoString;


namespace leopph
{
	class Component;
	class Transform;
	class Scene;


	class Entity : public ManagedAccessObject
	{
	public:
		std::string name;
		std::vector<std::unique_ptr<Component>> components;
		Scene* scene;
		Transform* transform;

		static Entity* Create();
		void AssignManagedObject(MonoObject* managedObject);

		template<std::derived_from<Component> T>
		T* GetComponent() const
		{
			for (auto const& component : components)
			{
				if (auto const castPtr = dynamic_cast<T*>(component.get()))
				{
					return castPtr;
				}
			}

			return nullptr;
		}

		template<std::derived_from<Component> T>
		void GetComponents(std::vector<T*>& outComponents) const
		{
			for (auto const& component : components)
			{
				if (auto const castPtr = dynamic_cast<T*>(component.get()))
				{
					outComponents.emplace_back(castPtr);
				}
			}
		}
	};


	LEOPPHAPI extern std::vector<std::unique_ptr<Entity>> gEntities;

	void DestroyEntity(Entity* const entity);
	Entity* FindEntity(std::string_view name);


	namespace managedbindings
	{
		void CreateNativeEntity(MonoObject* managedEntity);
		MonoObject* GetEntityTransform(MonoObject* managedEntity);
		MonoString* GetEntityName(MonoObject* managedEntity);
		void SetEntityName(MonoObject* managedEntity, MonoString* managedName);
	}
}