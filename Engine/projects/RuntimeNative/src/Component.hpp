#pragma once

#include "ManagedAccessObject.hpp"

struct _MonoReflectionType;
typedef _MonoReflectionType MonoReflectionType;


namespace leopph
{
	class Entity;


	class Component : public ManagedAccessObject
	{
	public:
		Entity* const entity;

		Component(MonoObject* managedObject, Entity* entity);
		~Component() override;
	};

	
	namespace managedbindings
	{
		MonoObject* create_component(MonoReflectionType* refType, Entity* entity);
		u64 component_get_entity_handle(Component const* component);
	}
}