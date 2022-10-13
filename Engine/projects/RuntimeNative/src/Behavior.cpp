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
		std::unordered_map<Behavior*, MonoMethod*> gToInit;
		std::unordered_map<Behavior*, MonoMethod*> gToTick;
		std::unordered_map<Behavior*, MonoMethod*> gToTack;


		void invoke_method_handle_exception(MonoObject* const obj, MonoMethod* const method)
		{
			MonoObject* exception;
			mono_runtime_invoke(method, obj, nullptr, &exception);

			if (exception)
			{
				mono_print_unhandled_exception(exception);
			}
		}
	}


	Behavior::Behavior(MonoObject* const managedObject, Entity* const entity, MonoClass* const klass) :
		Component{ managedObject, entity }
	{
		if (MonoMethod* const initMethod = mono_class_get_method_from_name(klass, "OnInit", 0))
		{
			gToInit[this] = initMethod;
		}

		if (MonoMethod* const tickMethod = mono_class_get_method_from_name(klass, "Tick", 0))
		{
			gToTick[this] = tickMethod;
		}

		if (MonoMethod* const tackMethod = mono_class_get_method_from_name(klass, "Tack", 0))
		{
			gToTack[this] = tackMethod;
		}

		if (MonoMethod* const ctor = mono_class_get_method_from_name(klass, ".ctor", 0))
		{
			invoke_method_handle_exception(managedObject, ctor);
		}
	}


	Behavior::~Behavior()
	{
		gToInit.erase(this);
		gToTick.erase(this);
		gToTack.erase(this);

		MonoObject* const managedObj = mono_gchandle_get_target(managedObjectHandle);

		if (MonoMethod* const destroyMethod = mono_class_get_method_from_name(mono_object_get_class(managedObj), "OnDestroy", 0))
		{
			invoke_method_handle_exception(managedObj, destroyMethod);
		}

		entity->remove_component(this);
	}


	void init_behaviors()
	{
		for (auto const& [behavior, method] : gToInit)
		{
			invoke_method_handle_exception(mono_gchandle_get_target(behavior->managedObjectHandle), method);
		}

		gToInit.clear();
	}


	void tick_behaviors()
	{
		for (auto const& [behavior, method] : gToTick)
		{
			invoke_method_handle_exception(mono_gchandle_get_target(behavior->managedObjectHandle), method);
		}
	}


	void tack_behaviors()
	{
		for (auto const& [behavior, method] : gToTack)
		{
			invoke_method_handle_exception(mono_gchandle_get_target(behavior->managedObjectHandle), method);
		}
	}
}