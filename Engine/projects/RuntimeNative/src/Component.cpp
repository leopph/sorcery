#include "Component.hpp"

#include "Entity.hpp"


namespace leopph
{
	Component::Component(u64 const managedObjectHandle, Entity* const entity) :
		ManagedAccessObject{ managedObjectHandle }, entity{ entity }
	{
		entity->add_component(this);
	}


	Component::~Component()
	{
		entity->remove_component(this);
	}


	namespace managedbindings
	{
		u64 component_get_entity_handle(Component const* const component)
		{
			return component->entity->managedObjectHandle;
		}
	}
}