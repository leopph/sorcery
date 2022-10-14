#include "Camera.hpp"

namespace leopph
{
	Camera* Camera::sInstance{ nullptr };

	Camera::Camera(MonoObject* const managedObject, Entity* const entity) :
		Component{ managedObject, entity }
	{
		sInstance = this;
	}


	Camera::~Camera()
	{
		sInstance = nullptr;
	}


	Camera* Camera::get_instance()
	{
		return sInstance;
	}
}