#pragma once

#include "ShadowMap.hpp"
#include "../events/EventReceiver.hpp"
#include "../events/SpotShadowMapResChangedEvent.hpp"
#include "shaders/ShaderProgram.hpp"



namespace leopph::impl
{
	class SpotLightShadowMap final : public ShadowMap, EventReceiver<SpotShadowMapResChangedEvent>
	{
		public:
			SpotLightShadowMap();

			[[nodiscard]] int BindForReading(ShaderProgram& shader, int textureUnit) const;


		private:
			void OnEventReceived(EventParamType event) override;
	};
}