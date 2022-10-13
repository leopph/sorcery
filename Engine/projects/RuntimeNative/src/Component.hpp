#pragma once

#include "ManagedAccessObject.hpp"

namespace leopph
{
	class Entity;


	class Component : public ManagedAccessObject
	{
	public:
		Entity* const entity;

		Component(u64 managedObjectHandle, Entity* entity);
		~Component() override;
	};

	
	namespace managedbindings
	{
		u64 component_get_entity_handle(Component const* component);
	}
}