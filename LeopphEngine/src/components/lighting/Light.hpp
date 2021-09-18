#pragma once

#include "../Component.hpp"
#include "../../api/leopphapi.h"
#include "../../math/Vector.hpp"



namespace leopph::impl
{
	class Light : public Component
	{
		public:
			LEOPPHAPI explicit Light(Entity& owner);
			LEOPPHAPI ~Light() override = 0;

			Light(const Light&) = delete;
			Light(Light&&) = delete;
			void operator=(const Light&) = delete;
			void operator=(Light&&) = delete;

			LEOPPHAPI const Vector3& Diffuse() const;
			LEOPPHAPI const Vector3& Specular() const;

			LEOPPHAPI void Diffuse(const Vector3& value);
			LEOPPHAPI void Specular(const Vector3& value);

			LEOPPHAPI float Range() const;
			LEOPPHAPI void Range(float value);

			LEOPPHAPI bool CastsShadow() const;
			LEOPPHAPI void CastsShadow(bool value);


		private:
			bool m_CastsShadow;
			float m_Range;
			Vector3 m_Diffuse;
			Vector3 m_Specular;
	};
}
