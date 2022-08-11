#pragma once

#include "AttenuatedLight.hpp"
#include "Types.hpp"


namespace leopph
{
	class SpotLight final : public AttenuatedLight
	{
		public:
			[[nodiscard]] LEOPPHAPI f32 get_inner_angle() const;
			LEOPPHAPI void set_inner_angle(f32 degrees);

			[[nodiscard]] LEOPPHAPI f32 get_outer_angle() const;
			LEOPPHAPI void set_outer_angle(f32 degrees);


			LEOPPHAPI SpotLight();

			SpotLight(SpotLight const& other) = delete;
			SpotLight& operator=(SpotLight const& other) = delete;

			SpotLight(SpotLight&& other) = delete;
			void operator=(SpotLight&& other) = delete;

			LEOPPHAPI ~SpotLight() override;


		private:
			float mInnerAngle{30.f};
			float mOuterAngle{30.f};
	};
}
