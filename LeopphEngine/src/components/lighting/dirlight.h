#pragma once

#include "../../api/leopphapi.h"
#include "light.h"
#include "../../math/vector.h"

namespace leopph
{
	class DirectionalLight : public impl::Light
	{
	public:
		LEOPPHAPI DirectionalLight(leopph::Object& object);

		LEOPPHAPI void Direction(const Vector3& newDir);
		LEOPPHAPI const Vector3& Direction() const;

	private:
		Vector3 m_Direction{ 1.0f, 1.0f, 1.0f };
	};
}