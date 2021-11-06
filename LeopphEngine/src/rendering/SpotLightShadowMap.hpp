#pragma once

#include "ShadowMap.hpp"
#include "../events/SpotShadowResolutionEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "shaders/ShaderProgram.hpp"



namespace leopph::impl
{
	class SpotLightShadowMap final : public ShadowMap, EventReceiver<SpotShadowResolutionEvent>
	{
		public:
			SpotLightShadowMap();

			[[nodiscard]] int BindForReading(ShaderProgram& shader, int textureUnit) const;


		private:
			void OnEventReceived(EventParamType event) override;
	};
}