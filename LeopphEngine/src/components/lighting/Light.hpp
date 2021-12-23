#pragma once

#include "../Component.hpp"
#include "../../api/LeopphApi.hpp"
#include "../../math/Vector.hpp"


namespace leopph::internal
{
	// Base class for all lights.
	class Light : public Component
	{
		public:
			/* Get the current diffuse intensity and color.
			 * This value is in the [0; 1] range per component. */
			[[nodiscard]]
			LEOPPHAPI auto Diffuse() const -> const Vector3&;

			/* Set the current diffuse intensity and color.
			 * This value must be in the [0; 1] range per component. */
			LEOPPHAPI auto Diffuse(const Vector3& value) -> void;

			/* Get the current specular highlight intensity and color.
			 * This value is in the [0; 1] range per component. */
			[[nodiscard]]
			LEOPPHAPI auto Specular() const -> const Vector3&;

			/* Set the current specular highlight intensity and color.
			 * This value must be in the [0; 1] range per component. */
			LEOPPHAPI auto Specular(const Vector3& value) -> void;

			/* Get the distance where the Light's effect fully cuts off. */
			[[nodiscard]]
			LEOPPHAPI auto Range() const -> float;

			/* Set the distance where the Light's effect fully cuts off. */
			LEOPPHAPI auto Range(float value) -> void;

			/* Get whether the Light's effect can be occluded by objects.
			 * This only works if objects have this property set to true.
			 * This value is false by default. */
			[[nodiscard]]
			LEOPPHAPI auto CastsShadow() const -> bool;

			/* Set whether the Light's effect can be occluded by objects.
			 * This only works if objects have this property set to true.
			 * This value is false by default. */
			LEOPPHAPI auto CastsShadow(bool value) -> void;

			LEOPPHAPI explicit Light(leopph::Entity* entity);

			Light(const Light&) = delete;
			auto operator=(const Light&) -> void = delete;

			Light(Light&&) = delete;
			auto operator=(Light&&) -> void = delete;

			LEOPPHAPI ~Light() override = 0;

		private:
			bool m_CastsShadow;
			float m_Range;
			Vector3 m_Diffuse;
			Vector3 m_Specular;
	};
}
