#pragma once

#include <set>

#include "vector.h"
#include "behavior.h"

namespace leopph::implementation
{
	class Light : public Component
	{
	public:
		using Component::Component;
		virtual ~Light() = 0;

		static const std::set<Light*>& PointLights();
		static Light* DirectionalLight();

		const Vector3& Ambient() const;
		const Vector3& Diffuse() const;
		const Vector3& Specular() const;

		void Ambient(const Vector3& value);
		void Diffuse(const Vector3& value);
		void Specular(const Vector3& value);

	protected:
		static void RegisterPointLight(Light* light);
		static void RegisterDirectionalLight(Light* light);

		static void UnregisterPointLight(Light* light);
		static void UnregisterDirectionalLight();

	private:
		static std::set<Light*> s_PointLights;
		static Light* s_DirectionalLight;

		Vector3 m_Ambient{ 0.05f, 0.05f, 0.05f };
		Vector3 m_Diffuse{ 0.8f, 0.8f, 0.8f };
		Vector3 m_Specular{ 1.0f, 1.0f, 1.0f };
	};
}