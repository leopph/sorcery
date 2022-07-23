#include "AmbientLight.hpp"



leopph::AmbientLight& leopph::AmbientLight::Instance()
{
	static AmbientLight instance;
	return instance;
}
