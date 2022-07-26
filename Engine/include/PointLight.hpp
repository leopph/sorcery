#pragma once

#include "AttenuatedLight.hpp"


namespace leopph
{
	class PointLight final : public AttenuatedLight
	{
		public:
			LEOPPHAPI void Owner(Entity* entity) override;
			using AttenuatedLight::Owner;


			LEOPPHAPI void Active(bool active) override;
			using AttenuatedLight::Active;


			[[nodiscard]] LEOPPHAPI ComponentPtr<> Clone() const override;


			PointLight() = default;

			PointLight(PointLight const& other) = default;
			LEOPPHAPI PointLight& operator=(PointLight const& other);

			PointLight(PointLight&& other) = delete;
			void operator=(PointLight&& other) = delete;

			LEOPPHAPI ~PointLight() override;
	};
}
