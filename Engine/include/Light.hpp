#pragma once

#include "Vector.hpp"
#include "Component.hpp"


namespace leopph
{
	// Base class for all lights.
	class Light : public Component
	{
		public:
			// Get the current diffuse intensity and color.
			// This value is in the [0; 1] range per component.
			[[nodiscard]] auto LEOPPHAPI Diffuse() const noexcept -> Vector3 const&;

			// Set the current diffuse intensity and color.
			// This value must be in the [0; 1] range per component.
			auto LEOPPHAPI Diffuse(Vector3 const& value) noexcept -> void;


			// Get the current specular highlight intensity and color.
			// This value is in the [0; 1] range per component.
			[[nodiscard]] auto LEOPPHAPI Specular() const noexcept -> Vector3 const&;

			// Set the current specular highlight intensity and color.
			// This value must be in the [0; 1] range per component.
			auto LEOPPHAPI Specular(Vector3 const& value) noexcept -> void;


			// Get whether the Light's effect can be occluded by objects.
			// This only works if objects have this property set to true.
			// This value is false by default.
			[[nodiscard]] auto LEOPPHAPI CastsShadow() const noexcept -> bool;

			// Set whether the Light's effect can be occluded by objects.
			// This only works if objects have this property set to true.
			// This value is false by default.
			auto LEOPPHAPI CastsShadow(bool value) noexcept -> void;

		protected:
			using Component::Component;
			using Component::operator=;

		private:
			bool m_CastsShadow{false};
			Vector3 m_Diffuse{1.f, 1.f, 1.f};
			Vector3 m_Specular{1.f, 1.f, 1.f};
	};
}
