#pragma once

#include "Core.hpp"
#include "Component.hpp"


namespace leopph
{
	class CubeModel : public Component
	{
	public:
		CubeModel(MonoObject* managedObject, Entity* entity);
		~CubeModel() override;
	};
}