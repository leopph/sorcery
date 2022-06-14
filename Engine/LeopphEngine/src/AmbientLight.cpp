#include "AmbientLight.hpp"


auto leopph::AmbientLight::Instance() -> leopph::AmbientLight&
{
	static AmbientLight instance;
	return instance;
}
