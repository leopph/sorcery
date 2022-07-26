#pragma once

#include "Component.hpp"
#include "Vector.hpp"


namespace leopph
{
	class Light : public Component
	{
		public:
			[[nodiscard]] LEOPPHAPI Vector3 const& get_color() const;
			LEOPPHAPI void set_color(Vector3 const& newColor);

			[[nodiscard]] LEOPPHAPI f32 get_intensity() const;
			LEOPPHAPI void set_intensity(f32 newIntensity);

			[[nodiscard]] LEOPPHAPI bool is_casting_shadow() const;
			LEOPPHAPI void set_casting_shadow(bool newValue);

		protected:
			using Component::Component;
			using Component::operator=;

		private:
			bool mCastsShadow{false};
			Vector3 mColor{1.f};
			f32 mIntensity{1.f};
	};
}
