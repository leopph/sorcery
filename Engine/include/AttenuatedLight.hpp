#pragma once

#include "Light.hpp"
#include "Types.hpp"


namespace leopph
{
	class AttenuatedLight : public Light
	{
		public:
			[[nodiscard]] LEOPPHAPI f32 get_range() const;
			LEOPPHAPI void set_range(f32 value);


		protected:
			using Light::Light;
			using Light::operator=;


		private:
			f32 mRange{10.f};
	};
}
