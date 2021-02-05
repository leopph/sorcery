#pragma once

#include "light.h"
#include "behavior.h"
#include "vector.h"

namespace leopph
{
#pragma warning(push)
#pragma warning (disable: 4275)
#pragma warning (disable: 4251)

	// POINT LIGHT SOURCE
	class LEOPPHAPI PointLight final : public implementation::Light
	{
	public:
		using implementation::Light::Light;

		void operator()() {}


		float Range() const;
		
		float Constant() const;
		float Linear() const;
		float Quadratic() const;

		const Vector3& Ambient() const;
		const Vector3& Diffuse() const;
		const Vector3& Specular() const;

		void Constant(float value);
		void Linear(float value);
		void Quadratic(float value);

		void Ambient(const Vector3& value);
		void Diffuse(const Vector3& value);
		void Specular(const Vector3& value);

	private:
		float m_Constant{ 1.0f };
		float m_Linear{ 0.22f };
		float m_Quadratic{ 0.2f };

		Vector3 m_Ambient{ 0.05f, 0.05f, 0.05f };
		Vector3 m_Diffuse{ 0.8f, 0.8f, 0.8f };
		Vector3 m_Specular{ 1.0f, 1.0f, 1.0f };
	};

#pragma warning(pop)
}