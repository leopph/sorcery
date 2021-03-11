#pragma once

#include "leopphapi.h"
#include "light.h"
#include "vector.h"

namespace leopph
{
#pragma warning(push)
#pragma warning (disable: 4275)
#pragma warning (disable: 4251)

	class LEOPPHAPI DirectionalLight : public implementation::Light
	{
	public:
		DirectionalLight(leopph::Object& object);

		void Direction(const Vector3& newDir);
		const Vector3& Direction() const;

	private:
		Vector3 m_Direction{ 1.0f, 1.0f, 1.0f };
	};

#pragma warning(pop)
}