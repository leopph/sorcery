#pragma once

#include "../../math/vector.h"
#include "../behavior.h"

namespace leopph::impl
{
	class Light : public Component
	{
	public:
		using Component::Component;
		~Light() override = 0;

		const Vector3& Ambient() const;
		const Vector3& Diffuse() const;
		const Vector3& Specular() const;

		void Ambient(const Vector3& value);
		void Diffuse(const Vector3& value);
		void Specular(const Vector3& value);

	private:
		Vector3 m_Ambient{ 0.05f, 0.05f, 0.05f };
		Vector3 m_Diffuse{ 0.8f, 0.8f, 0.8f };
		Vector3 m_Specular{ 1.0f, 1.0f, 1.0f };
	};
}