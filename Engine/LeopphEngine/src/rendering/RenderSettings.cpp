#include "RenderSettings.hpp"


namespace leopph
{
	u16 RenderSettings::get_directional_light_shadow_map_size()
	{
		return mDirShadowMapSize;
	}



	u16 RenderSettings::get_punctual_lights_shadow_map_size()
	{
		return mPuncShadowMapSize;
	}



	u8 RenderSettings::get_shadow_cascade_count()
	{
		return mNumShadowCascades;
	}



	u8 RenderSettings::get_max_spot_light_count()
	{
		return mNumMaxSpotLights;
	}



	u8 RenderSettings::get_max_point_light_count()
	{
		return mNumMaxPointLights;
	}
}
