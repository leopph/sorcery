#pragma once

#include "LeopphApi.hpp"
#include "Light.hpp"
#include "Types.hpp"


namespace leopph
{
	class DirectionalLight final : public Light
	{
		public:
			[[nodiscard]] LEOPPHAPI Vector3 const& get_direction() const;


			[[nodiscard]] LEOPPHAPI f32 get_shadow_near_plane() const;
			LEOPPHAPI void set_shadow_near_plane(f32 newVal);


			LEOPPHAPI DirectionalLight();

			DirectionalLight(DirectionalLight const& other) = delete;
			DirectionalLight& operator=(DirectionalLight const& other) = delete;

			DirectionalLight(DirectionalLight&& other) = delete;
			void operator=(DirectionalLight&& other) = delete;

			LEOPPHAPI ~DirectionalLight() override;

		private:
			f32 mShadowNear{50.f};
	};
}
