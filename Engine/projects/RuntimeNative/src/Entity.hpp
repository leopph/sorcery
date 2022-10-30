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
	};


	LEOPPHAPI extern std::vector<std::unique_ptr<Entity>> gEntities;

	void DestroyEntity(Entity* const entity);
	Entity* FindEntity(std::string_view name);


	namespace managedbindings
	{
		void CreateNativeEntity(MonoObject* managedEntity);
		MonoObject* GetEntityTransform(MonoObject* managedEntity);
	}
}