#include "color.h"

leopph::Color::operator leopph::Vector3() const
{
	return Vector3{ red / 255.f, green / 255.f, blue / 255.f };
}
