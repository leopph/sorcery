#pragma once

#include "Core.hpp"

#include "Component.hpp"

#include <vector>

struct _MonoClass;
typedef _MonoClass MonoClass;

namespace leopph
{
	class Entity;

	class Behavior : public Component
	{
	public:
		Behavior(MonoObject* managedObject, Entity* entity, MonoClass* klass);
		~Behavior() override;
	};

	LEOPPHAPI void init_behaviors();
	LEOPPHAPI void tick_behaviors();
	LEOPPHAPI void tack_behaviors();
}