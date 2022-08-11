#pragma once

#include "AttenuatedLight.hpp"


namespace leopph
{
	class PointLight final : public AttenuatedLight
	{
		public:
			LEOPPHAPI PointLight();

			PointLight(PointLight const& other) = delete;
			PointLight& operator=(PointLight const& other) = delete;

			PointLight(PointLight&& other) = delete;
			void operator=(PointLight&& other) = delete;

			LEOPPHAPI ~PointLight() override;
	};
}
