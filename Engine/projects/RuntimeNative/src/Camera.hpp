#pragma once

#include "Component.hpp"

namespace leopph
{
	class Camera : public Component
	{
	private:
		static Camera* sInstance;

	public:
		Camera(MonoObject* managedObject, Entity* entity);
		~Camera() override;
		[[nodiscard]] static Camera* get_instance();
	};
}