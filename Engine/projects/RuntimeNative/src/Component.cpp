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

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <concepts>


namespace leopph
{
	namespace
	{
		template<std::derived_from<Component> T>
		Component* instantiate(MonoObject* const managedObject, Entity* const entity)
		{
			return new T{ managedObject, entity };
		}

		std::unordered_map<std::string, std::unordered_map<std::string, std::function<Component*(MonoObject*, Entity*)>>> const gComponentInstantiators
		{
			{"leopph", {
				{"CubeModel", instantiate<CubeModel>}
		}
			}
		};
	}

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
				return managedObject;
			}

			auto* const className = mono_class_get_name(managedClass);
			auto* const namespaceName = mono_class_get_namespace(managedClass);

			if (auto const nsIt = gComponentInstantiators.find(namespaceName); nsIt != std::end(gComponentInstantiators))
			{
				if (auto const nameIt = nsIt->second.find(className); nameIt != std::end(nsIt->second))
				{
					store_mao(nameIt->second(managedObject, entity));
					return managedObject;
				}
			}

			MessageBoxW(nullptr, L"Failed to find component instantiator.", L"Error", MB_ICONERROR);
			return nullptr;
		}


		u64 component_get_entity_handle(Component const* const component)
		{
			return component->entity->managedObjectHandle;
		}
	}
}