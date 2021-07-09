#pragma once

#include "../behavior.h"
#include "../../api/leopphapi.h"
#include "../../math/vector.h"

namespace leopph::impl
{
	class Light : public Component
	{
	public:
		Light();
		~Light() override = 0;

		LEOPPHAPI const Vector3& Diffuse() const;
		LEOPPHAPI const Vector3& Specular() const;

		LEOPPHAPI void Diffuse(const Vector3& value);
		LEOPPHAPI void Specular(const Vector3& value);

	private:
		Vector3 m_Diffuse;
		Vector3 m_Specular;
	};
}