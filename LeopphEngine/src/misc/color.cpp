#include "Color.hpp"


namespace leopph
{
	Color::operator Vector3() const
	{
		return Vector3{static_cast<float>(red) / 255.f, static_cast<float>(green) / 255.f, static_cast<float>(blue) / 255.f};
	}
}
