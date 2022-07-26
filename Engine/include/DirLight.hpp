#pragma once

#include "LeopphApi.hpp"
#include "Light.hpp"
#include "Types.hpp"


namespace leopph
{
	class DirectionalLight final : public Light
	{
		public:
			[[nodiscard]] LEOPPHAPI Vector3 const& get_direction() const noexcept;

			[[nodiscard]] LEOPPHAPI f32 get_shadow_near_plane() const noexcept;
			LEOPPHAPI void set_shadow_near_plane(f32 newVal);


			LEOPPHAPI void Owner(Entity* entity) override;
			using Light::Owner;


			LEOPPHAPI void Active(bool active) override;
			using Light::Active;


			[[nodiscard]] LEOPPHAPI ComponentPtr<> Clone() const override;

			DirectionalLight() = default;

			DirectionalLight(DirectionalLight const& other) = default;
			LEOPPHAPI DirectionalLight& operator=(DirectionalLight const& other);

			DirectionalLight(DirectionalLight&& other) = delete;
			void operator=(DirectionalLight&& other) = delete;

			LEOPPHAPI ~DirectionalLight() override;

		private:
			f32 mShadowNear{50.f};
	};
}
