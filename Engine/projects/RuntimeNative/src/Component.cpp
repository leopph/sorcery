#include "Component.hpp"

#include "Entity.hpp"
#include "Managed.hpp"
#include "Behavior.hpp"
#include "CubeModel.hpp"

#include <mono/metadata/appdomain.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/image.h>
#include <mono/metadata/reflection.h>


namespace leopph
{
	Component::Component(MonoObject* const managedObject, Entity* const entity) :
		ManagedAccessObject{ managedObject }, entity{ entity }
	{
		entity->add_component(this);
	}


	Component::~Component()
	{
		entity->remove_component(this);
	}


	namespace managedbindings
	{
		MonoObject* create_component(MonoReflectionType* refType, Entity* entity)
		{
			MonoClass* const managedClass = mono_class_from_mono_type(mono_reflection_type_get_type(refType));
			MonoImage* const managedImage = mono_class_get_image(managedClass);
			MonoObject* const managedObject = mono_object_new(mono_domain_get(), managedClass);

			if (static MonoClass* const behaviorClass = mono_class_from_name(managedImage, "leopph", "Behavior");
				mono_class_is_subclass_of(managedClass, behaviorClass, false))
			{
				store_mao(new Behavior{ managedObject, entity, managedClass });
			}
			else
			{
				// TODO generic component instantiation
				auto* const className = mono_class_get_name(managedClass);
				auto* const namespaceName = mono_class_get_namespace(managedClass);

				if (!std::strcmp(className, "CubeModel") && !std::strcmp(namespaceName, "leopph"))
				{
					store_mao(new CubeModel{ managedObject, entity });
				}
			}

			return managedObject;
		}


		u64 component_get_entity_handle(Component const* const component)
		{
			return component->entity->managedObjectHandle;
		}
	}
}