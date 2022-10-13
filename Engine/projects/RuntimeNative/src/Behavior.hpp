#pragma once

#include "Core.hpp"

#include "Component.hpp"

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

	class Behavior : public Component
	{
	public:
		MonoClass* const klass;
		MonoMethod* const initFunc;
		MonoMethod* const tickFunc;
		MonoMethod* const tackFunc;
		MonoMethod* const destroyFunc;

		Behavior(u64 managedObjectHandle, Entity* entity, MonoClass* klass, MonoMethod* initFunc, MonoMethod* tickFunc, MonoMethod* tackFunc, MonoMethod* destroyFunc);
		~Behavior() override;
	};

	LEOPPHAPI void init_behaviors();
	LEOPPHAPI void tick_behaviors();
	LEOPPHAPI void tack_behaviors();

	namespace managedbindings
	{
		u64 behavior_new(MonoReflectionType* refType, Entity* entity);
	}
}