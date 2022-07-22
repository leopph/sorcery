#pragma once

#include "Component.hpp"
#include "Vector.hpp"


namespace leopph
{
	// Base class for all lights.
	class Light : public Component
	{
		public:
			// Get the current diffuse intensity and color.
			// This value is in the [0; 1] range per component.
			[[nodiscard]] LEOPPHAPI Vector3 const& Diffuse() const noexcept;

			// Set the current diffuse intensity and color.
			// This value must be in the [0; 1] range per component.
			LEOPPHAPI void Diffuse(Vector3 const& value) noexcept;


			// Get the current specular highlight intensity and color.
			// This value is in the [0; 1] range per component.
			[[nodiscard]] LEOPPHAPI Vector3 const& Specular() const noexcept;

			// Set the current specular highlight intensity and color.
			// This value must be in the [0; 1] range per component.
			LEOPPHAPI void Specular(Vector3 const& value) noexcept;


			// Get whether the Light's effect can be occluded by objects.
			// This only works if objects have this property set to true.
			// This value is false by default.
			[[nodiscard]] LEOPPHAPI bool CastsShadow() const noexcept;

			// Set whether the Light's effect can be occluded by objects.
			// This only works if objects have this property set to true.
			// This value is false by default.
			LEOPPHAPI void CastsShadow(bool value) noexcept;

		protected:
			using Component::Component;
			using Component::operator=;

		private:
			bool m_CastsShadow{false};
			Vector3 m_Diffuse{1.f, 1.f, 1.f};
			Vector3 m_Specular{1.f, 1.f, 1.f};
	};
}
