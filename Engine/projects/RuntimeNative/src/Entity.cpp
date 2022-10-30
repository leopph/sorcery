#include "Entity.hpp"

#include "Components.hpp"
#include "SceneManager.hpp"
#include "ManagedRuntime.hpp"

#include <mono/metadata/class.h>
#include <mono/metadata/reflection.h>


namespace leopph
{
	std::vector<std::unique_ptr<Entity>> gEntities;

	namespace
	{
		[[nodiscard]] Entity* CreateAndSetupEntityAndTransform()
		{
			auto const entity = gEntities.emplace_back(std::make_unique<Entity>()).get();
			entity->name = "Entity";
			entity->scene = SceneManager::GetActiveScene();
			entity->scene->AddEntity(entity);

			auto const transform = new Transform{ entity };
			entity->components.emplace_back(transform);
			entity->transform = transform;

			return entity;
		}
	}


	Entity* Entity::Create()
	{
		auto const entity = CreateAndSetupEntityAndTransform();

		static auto const entityClass = mono_class_from_name(GetManagedImage(), "leopph", "Entity");
		entity->CreateManagedObject(entityClass);

		return entity;
	}


	void Entity::AssignManagedObject(MonoObject* const managedObject)
	{
		SetManagedObject(managedObject);
	}


	void DestroyEntity(Entity* const entity)
	{
		entity->scene->RemoveEntity(entity);
		std::erase_if(gEntities, [entity](auto const& other)
		{
			return other.get() == entity;
		});
	}


	Entity* FindEntity(std::string_view const name)
	{
		for (auto const& entity : gEntities)
		{
			if (entity->name == name)
			{
				return entity.get();
			}
		}

		return nullptr;
	}

	
	namespace managedbindings
	{
		void CreateNativeEntity(MonoObject* managedObject)
		{
			auto const entity = CreateAndSetupEntityAndTransform();
			entity->AssignManagedObject(managedObject);
		}


		MonoObject* GetEntityTransform(MonoObject* managedEntity)
		{
			auto const nativeEntity = ManagedAccessObject::GetNativePtrFromManagedObjectAs<Entity*>(managedEntity);
			return nativeEntity->transform->GetManagedObject();
		}
	}
}