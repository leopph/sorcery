#include "Behavior.hpp"

#include "Entity.hpp"

#include <mono/metadata/appdomain.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

#include <cassert>
#include <unordered_map>
#include <memory>
#include <vector>

namespace leopph
{
	namespace
	{
		std::vector<Behavior*> gToInit;
		std::vector<Behavior*> gToTick;
		std::vector<Behavior*> gToTack;
	}


	Behavior::Behavior(u64 const managedObjectHandle, MonoClass* const klass, MonoMethod* const initFunc, MonoMethod* const tickFunc, MonoMethod* const tackFunc, MonoMethod* const destroyFunc, Entity* const entity) :
		ManagedAccessObject{ managedObjectHandle }, klass{ klass }, initFunc{ initFunc }, tickFunc{ tickFunc }, tackFunc{ tackFunc }, destroyFunc{ destroyFunc }, entity{ entity }
	{}


	Behavior::~Behavior()
	{
		std::erase(gToInit, this);
		std::erase(gToTick, this);
		std::erase(gToTack, this);

		if (destroyFunc)
		{
			MonoObject* exception;
			mono_runtime_invoke(destroyFunc, mono_gchandle_get_target(static_cast<u32>(managedObjectHandle)), nullptr, &exception);

			if (exception)
			{
				mono_print_unhandled_exception(exception);
			}
		}

		entity->remove_behavior(this);
	}


	void init_behaviors()
	{
		for (Behavior* const behavior : gToInit)
		{
			MonoObject* exception;
			mono_runtime_invoke(behavior->initFunc, mono_gchandle_get_target(static_cast<u32>(behavior->managedObjectHandle)), nullptr, &exception);

			if (exception)
			{
				mono_print_unhandled_exception(exception);
			}
		}

		gToInit.clear();
	}


	void tick_behaviors()
	{
		for (Behavior* const behavior : gToTick)
		{
			MonoObject* exception;
			mono_runtime_invoke(behavior->tickFunc, mono_gchandle_get_target(static_cast<u32>(behavior->managedObjectHandle)), nullptr, &exception);

			if (exception)
			{
				mono_print_unhandled_exception(exception);
			}
		}
	}


	void tack_behaviors()
	{
		for (Behavior* const behavior : gToTack)
		{
			MonoObject* exception;
			mono_runtime_invoke(behavior->tackFunc, mono_gchandle_get_target(static_cast<u32>(behavior->managedObjectHandle)), nullptr, &exception);

			if (exception)
			{
				mono_print_unhandled_exception(exception);
			}
		}
	}



	namespace detail
	{
		u64 behavior_new(MonoReflectionType* const refType, Entity* const entity)
		{
			MonoClass* const klass = mono_class_from_mono_type(mono_reflection_type_get_type(refType));

			MonoObject* const obj = mono_object_new(mono_domain_get(), klass);
			u32 const gcHandle = mono_gchandle_new(obj, false);

			MonoMethod* const initMethod = mono_class_get_method_from_name(klass, "OnInit", 0);
			MonoMethod* const tickMethod = mono_class_get_method_from_name(klass, "Tick", 0);
			MonoMethod* const tackMethod = mono_class_get_method_from_name(klass, "Tack", 0);
			MonoMethod* const destroyMethod = mono_class_get_method_from_name(klass, "OnDestroy", 0);

			Behavior* const behavior = new Behavior{ gcHandle, klass, initMethod, tickMethod, tackMethod, destroyMethod, entity };

			store_mao(behavior);

			if (initMethod)
			{
				gToInit.emplace_back(behavior);
			}

			if (tickMethod)
			{
				gToTick.emplace_back(behavior);
			}

			if (tackMethod)
			{
				gToTack.emplace_back(behavior);
			}

			if (MonoMethod* const ctor = mono_class_get_method_from_name(klass, ".ctor", 0))
			{
				MonoObject* exception;
				mono_runtime_invoke(ctor, mono_gchandle_get_target(static_cast<u32>(behavior->managedObjectHandle)), nullptr, &exception);

				if (exception)
				{
					mono_print_unhandled_exception(exception);
				}
			}

			u64 idData{ behavior->id };
			Behavior* ptrData{ behavior };
			mono_field_set_value(mono_gchandle_get_target(gcHandle), mono_class_get_field_from_name(klass, "_id"), &idData);
			mono_field_set_value(mono_gchandle_get_target(gcHandle), mono_class_get_field_from_name(klass, "_ptr"), &ptrData);

			entity->add_behavior(behavior);

			return behavior->managedObjectHandle;
		}


		u64 behavior_get_entity_handle(Behavior* const behavior)
		{
			return behavior->entity->managedObjectHandle;
		}
	}
}