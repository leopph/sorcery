#include "Light.hpp"

#include "Logger.hpp"

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
			internal::Logger::Instance().Warning(std::format("Ignoring attempt to set light intensity to {}. This value must be positive.", newIntensity));
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
}
