#include "Entity.hpp"

#include "Components.hpp"
#include "SceneManager.hpp"
#include "ManagedRuntime.hpp"

#include <mono/metadata/class.h>
#include <mono/metadata/reflection.h>
#include <mono/metadata/object.h>

#include <functional>


namespace leopph
{
	std::vector<std::unique_ptr<Entity>> gEntities;

	namespace
	{
		template<std::derived_from<Component> T>
		Component* instantiate(Entity* const entity)
		{
			return new T{ entity };
		}


		std::unordered_map<std::string, std::unordered_map<std::string, std::function<Component* (Entity*)>>> const gComponentInstantiators
		{
			{"leopph",
				{
					{"CubeModel", instantiate<CubeModel>},
					{"Camera", instantiate<Camera>}
				}
			}
		};
	}


	Entity* Entity::Create()
	{
		auto const entity = gEntities.emplace_back(new Entity{}).get();
		entity->name = "Entity";
		entity->scene = SceneManager::GetActiveScene();
		entity->scene->AddEntity(entity);
		return entity;
	}


	Transform& Entity::GetTransform()
	{
		return *mTransform;
	}


	Component* Entity::CreateComponent(MonoClass* componentClass)
	{
		if (mono_class_is_subclass_of(componentClass, mono_class_from_name(GetManagedImage(), "leopph", "Behavior"), false))
		{
			auto const behavior = new Behavior{ this, componentClass };
			mComponents.emplace_back(behavior);
			return behavior;
		}

		auto* const classNs = mono_class_get_namespace(componentClass);
		auto* const className = mono_class_get_name(componentClass);

		if (!std::strcmp(classNs, "leopph") && !std::strcmp(className, "Transform"))
		{
			if (mTransform)
			{
				return nullptr;
			}

			// Transform is special because it needs to be cached in the entity
			return CreateComponent<Transform>();
		}

		if (auto const nsIt = gComponentInstantiators.find(classNs); nsIt != std::end(gComponentInstantiators))
		{
			if (auto const nameIt = nsIt->second.find(className); nameIt != std::end(nsIt->second))
			{
				auto const component = nameIt->second(this);
				component->CreateManagedObject(componentClass);
				mComponents.emplace_back(component);
				return component;
			}
		}

		return nullptr;
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
			auto const entity = Entity::Create();
			entity->SetManagedObject(managedObject);

			auto const transform = entity->CreateComponent<Transform>();
			transform->CreateManagedObject("leopph", "Transform");
		}


		MonoObject* GetEntityTransform(MonoObject* managedEntity)
		{
			auto const nativeEntity = ManagedAccessObject::GetNativePtrFromManagedObjectAs<Entity*>(managedEntity);
			return nativeEntity->GetTransform().GetManagedObject();
		}


		MonoString* GetEntityName(MonoObject* managedEntity)
		{
			return mono_string_new_wrapper(ManagedAccessObject::GetNativePtrFromManagedObjectAs<Entity*>(managedEntity)->name.c_str());
		}


		void SetEntityName(MonoObject* managedEntity, MonoString* managedName)
		{
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Entity*>(managedEntity)->name = mono_string_to_utf8(managedName);
		}


		MonoObject* EntityCreateComponent(Entity* const entity, MonoReflectionType* const componentType)
		{
			auto const componentClass = mono_type_get_class(mono_reflection_type_get_type(componentType));
			auto const component = entity->CreateComponent(componentClass);
			return component->GetManagedObject();
		}
	}
}