#pragma once

#include "Types.hpp"


namespace leopph
{
	class RenderSettings
	{
		public:
			[[nodiscard]] static u16 get_directional_light_shadow_map_size();
			[[nodiscard]] static u16 get_punctual_lights_shadow_map_size();
			[[nodiscard]] static u8 get_shadow_cascade_count();
			[[nodiscard]] static u8 get_max_spot_light_count();
			[[nodiscard]] static u8 get_max_point_light_count();


			RenderSettings() = delete;
			RenderSettings(RenderSettings const&) = delete;
			void operator=(RenderSettings const&) = delete;
			RenderSettings(RenderSettings&&) = delete;
			void operator=(RenderSettings&&) = delete;
			~RenderSettings() = delete;

		private:
			static u16 mDirShadowMapSize;
			static u16 mPuncShadowMapSize;
			static u8 mNumShadowCascades;
			static u8 mNumMaxSpotLights;
			static u8 mNumMaxPointLights;
	};
}
