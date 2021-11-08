#pragma once

#include "../Component.hpp"
#include "../../api/LeopphApi.hpp"
#include "../../math/Vector.hpp"


namespace leopph::impl
{
	// Base class for all lights.
	class Light : public Component
	{
		public:
			/* Get the current diffuse intensity and color.
			 * This value is in the [0; 1] range per component. */
			[[nodiscard]]
			LEOPPHAPI const Vector3& Diffuse() const;

			/* Set the current diffuse intensity and color.
			 * This value must be in the [0; 1] range per component. */
			LEOPPHAPI void Diffuse(const Vector3& value);

			/* Get the current specular highlight intensity and color.
			 * This value is in the [0; 1] range per component. */
			[[nodiscard]]
			LEOPPHAPI const Vector3& Specular() const;

			/* Set the current specular highlight intensity and color.
			 * This value must be in the [0; 1] range per component. */
			LEOPPHAPI void Specular(const Vector3& value);

			/* Get the distance where the Light's effect fully cuts off. */
			[[nodiscard]]
			LEOPPHAPI float Range() const;

			/* Set the distance where the Light's effect fully cuts off. */
			LEOPPHAPI void Range(float value);

			/* Get whether the Light's effect can be occluded by objects.
			 * This only works if objects have this property set to true.
			 * This value is false by default. */
			[[nodiscard]]
			LEOPPHAPI bool CastsShadow() const;

			/* Set whether the Light's effect can be occluded by objects.
			 * This only works if objects have this property set to true.
			 * This value is false by default. */
			LEOPPHAPI void CastsShadow(bool value);


			LEOPPHAPI explicit Light(leopph::Entity& owner);
			Light(const Light&) = delete;
			Light(Light&&) = delete;

			LEOPPHAPI ~Light() override = 0;

			void operator=(const Light&) = delete;
			void operator=(Light&&) = delete;


		private:
			bool m_CastsShadow;
			float m_Range;
			Vector3 m_Diffuse;
			Vector3 m_Specular;
	};
}
