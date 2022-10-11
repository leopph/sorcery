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
		std::unordered_map<u64, std::unique_ptr<Behavior>> gBehaviors;
		std::vector<Behavior*> gToInit;
		std::vector<Behavior*> gToTick;
		std::vector<Behavior*> gToTack;
	}


	void init_behaviors()
	{
		for (Behavior* const behavior : gToInit)
		{
			MonoObject* exception;
			mono_runtime_invoke(behavior->initFunc, mono_gchandle_get_target(behavior->gcHandle), nullptr, &exception);

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
			mono_runtime_invoke(behavior->tickFunc, mono_gchandle_get_target(behavior->gcHandle), nullptr, &exception);

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
			mono_runtime_invoke(behavior->tackFunc, mono_gchandle_get_target(behavior->gcHandle), nullptr, &exception);

			if (exception)
			{
				mono_print_unhandled_exception(exception);
			}
		}
	}



	namespace detail
	{
		MonoObject* create_behavior(MonoReflectionType* const refType, u64 const entityId)
		{
			MonoClass* const klass = mono_class_from_mono_type(mono_reflection_type_get_type(refType));

			MonoObject* const obj = mono_object_new(mono_domain_get(), klass);
			u32 const gcHandle = mono_gchandle_new(obj, false);

			MonoMethod* const initMethod = mono_class_get_method_from_name(klass, "OnInit", 0);
			MonoMethod* const tickMethod = mono_class_get_method_from_name(klass, "Tick", 0);
			MonoMethod* const tackMethod = mono_class_get_method_from_name(klass, "Tack", 0);
			MonoMethod* const destroyMethod = mono_class_get_method_from_name(klass, "OnDestroy", 0);

			static u64 nextId{1};
			u64 const id{nextId++};

			Entity* const entity = entities[id].get();

			Behavior* const behavior = new Behavior
			{
				.id = id,
				.entity = entity,
				.gcHandle = gcHandle,
				.klass = klass,
				.initFunc = initMethod,
				.tickFunc = tickMethod,
				.tackFunc = tackMethod,
				.destroyFunc = destroyMethod,
			};

			gBehaviors[id] = std::unique_ptr<Behavior>{behavior};

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
				mono_runtime_invoke(ctor, mono_gchandle_get_target(behavior->gcHandle), nullptr, &exception);

				if (exception)
				{
					mono_print_unhandled_exception(exception);
				}
			}

			u64 idData{id};
			mono_field_set_value(mono_gchandle_get_target(gcHandle), mono_class_get_field_from_name(klass, "_id"), &idData);

			entity->add_behavior(behavior);

			return mono_gchandle_get_target(behavior->gcHandle);
		}


		void destroy_behavior(u64 const id)
		{
			std::unique_ptr<Behavior>& behavior = gBehaviors[id];

			if (behavior->destroyFunc)
			{
				MonoObject* exception;
				mono_runtime_invoke(behavior->destroyFunc, mono_gchandle_get_target(behavior->gcHandle), nullptr, &exception);

				if (exception)
				{
					mono_print_unhandled_exception(exception);
				}
			}

			if (behavior->initFunc)
			{
				std::erase(gToInit, behavior.get());
			}

			if (behavior->tickFunc)
			{
				std::erase(gToTick, behavior.get());
			}

			if (behavior->tackFunc)
			{
				std::erase(gToTack, behavior.get());
			}

			behavior->entity->remove_behavior(behavior.get());

			behavior.reset();
		}


		u64 get_behavior_entity_id(u64 const behaviorId)
		{
			return gBehaviors[behaviorId]->entity->get_id();
		}
	}
}