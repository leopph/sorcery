#include "Lights.hpp"

#include "Context.hpp"
#include "Logger.hpp"
#include "Renderer.hpp"

#include <format>


namespace leopph
{
	Vector3 const& Light::get_color() const
	{
		return mColor;
	}



	void Light::set_color(Vector3 const& newColor)
	{
		mColor = newColor;
	}



	f32 Light::get_intensity() const
	{
		return mIntensity;
	}



	void Light::set_intensity(f32 const newIntensity)
	{
		if (newIntensity <= 0)
		{
			Logger::get_instance().warning(std::format("Ignoring attempt to set light intensity to {}. This value must be positive.", newIntensity));
			return;
		}

		mIntensity = newIntensity;
	}



	bool Light::is_casting_shadow() const
	{
		return mCastsShadow;
	}



	void Light::set_casting_shadow(bool const newValue)
	{
		mCastsShadow = newValue;
	}



	f32 AttenuatedLight::get_range() const
	{
		return mRange;
	}



	void AttenuatedLight::set_range(f32 const value)
	{
		mRange = value;
	}



	AmbientLight& AmbientLight::get_instance()
	{
		static AmbientLight instance;
		return instance;
	}



	Vector3 const& AmbientLight::get_intensity() const
	{
		return mIntensity;
	}



	void AmbientLight::set_intensity(Vector3 const& intensity)
	{
		mIntensity = intensity;
	}



	Vector3 const& DirectionalLight::get_direction() const
	{
		return get_forward_axis();
	}



	f32 DirectionalLight::get_shadow_near_plane() const
	{
		return mShadowNear;
	}



	void DirectionalLight::set_shadow_near_plane(f32 const newVal)
	{
		mShadowNear = newVal;
	}



	DirectionalLight::DirectionalLight()
	{
		internal::get_renderer().register_dir_light(this);
	}



	DirectionalLight::~DirectionalLight()
	{
		internal::get_renderer().unregister_dir_light(this);
	}



	PointLight::PointLight()
	{
		internal::get_renderer().register_point_light(this);
	}



	PointLight::~PointLight()
	{
		internal::get_renderer().unregister_point_light(this);
	}



	f32 SpotLight::get_inner_angle() const
	{
		return mInnerAngle;
	}



	void SpotLight::set_inner_angle(f32 const degrees)
	{
		mInnerAngle = degrees;
	}



	f32 SpotLight::get_outer_angle() const
	{
		return mOuterAngle;
	}



	void SpotLight::set_outer_angle(f32 const degrees)
	{
		mOuterAngle = degrees;
	}



	SpotLight::SpotLight()
	{
		internal::get_renderer().register_spot_light(this);
	}



	SpotLight::~SpotLight()
	{
		internal::get_renderer().unregister_spot_light(this);
	}
}
