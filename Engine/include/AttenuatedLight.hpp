#pragma once

#include "Light.hpp"
#include "Types.hpp"


namespace leopph
{
	// Attenuated lights are special Lights whose intensity decreases over distance.
	class AttenuatedLight : public Light
	{
		public:
			// Get the distance where the light effect fully cuts off.
			[[nodiscard]] LEOPPHAPI f32 Range() const noexcept;

			// Set the distance where the light effect fully cuts off.
			LEOPPHAPI void Range(f32 value) noexcept;

		protected:
			using Light::Light;
			using Light::operator=;

		private:
			f32 m_Range{10.f};
	};
}
