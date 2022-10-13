#pragma once

#include "Core.hpp"

#include "ManagedAccessObject.hpp"

#include <vector>

struct _MonoMethod;
struct _MonoClass;
struct _MonoReflectionType;

typedef _MonoMethod MonoMethod;
typedef _MonoClass MonoClass;
typedef _MonoReflectionType MonoReflectionType;

namespace leopph
{
	class Entity;

	class Behavior : public ManagedAccessObject
	{
	public:
		MonoClass* const klass;
		MonoMethod* const initFunc;
		MonoMethod* const tickFunc;
		MonoMethod* const tackFunc;
		MonoMethod* const destroyFunc;
		Entity* const entity;

		Behavior(u64 managedObjectHandle, MonoClass* klass, MonoMethod* initFunc, MonoMethod* tickFunc, MonoMethod* tackFunc, MonoMethod* destroyFunc, Entity* entity);
		~Behavior() override;
	};

	LEOPPHAPI void init_behaviors();
	LEOPPHAPI void tick_behaviors();
	LEOPPHAPI void tack_behaviors();

	namespace detail
	{
		u64 behavior_new(MonoReflectionType* refType, Entity* entity);
		u64 behavior_get_entity_handle(Behavior* behavior);
	}
}