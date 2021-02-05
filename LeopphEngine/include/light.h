#pragma once

#include <set>

#include "behavior.h"

namespace leopph::implementation
{
	class Light : public Behavior
	{
	public:
		using Behavior::Behavior;
		virtual ~Light() = 0;

		virtual void operator()() = 0;

		static const std::set<Light*>& PointLights();
		static Light* DirectionalLight();

	protected:
		static void RegisterPointLight(Light* light);
		static void RegisterDirectionalLight(Light* light);

		static void UnregisterPointLight(Light* light);
		static void UnregisterDirectionalLight(Light* light);

	private:
		static std::set<Light*> s_PointLights;
		static Light* s_DirectionalLight;
	};
}