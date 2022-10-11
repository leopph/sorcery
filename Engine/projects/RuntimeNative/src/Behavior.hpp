#pragma once

#include "Core.hpp"

#include <vector>

struct _MonoMethod;
struct _MonoObject;
struct _MonoClass;
struct _MonoReflectionType;

typedef _MonoMethod MonoMethod;
typedef _MonoObject MonoObject;
typedef _MonoClass MonoClass;
typedef _MonoReflectionType MonoReflectionType;

namespace leopph
{
	class Entity;

	struct Behavior
	{
		u64 id;
		Entity* entity;
		u32 gcHandle;
		MonoClass* klass;
		MonoMethod* initFunc;
		MonoMethod* tickFunc;
		MonoMethod* tackFunc;
		MonoMethod* destroyFunc;
	};

	LEOPPHAPI void init_behaviors();
	LEOPPHAPI void tick_behaviors();
	LEOPPHAPI void tack_behaviors();

	namespace detail
	{
		MonoObject* create_behavior(MonoReflectionType* refType, u64 entityId);
		void destroy_behavior(u64 id);
		u64 get_behavior_entity_id(u64 behaviorId);
	}
}