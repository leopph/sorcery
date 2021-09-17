#pragma once

#include "../math/Vector.hpp"

namespace leopph
{
	/*------------------------------------------------
	The Color struct represents an RGB color property.
	Its values are in the range [0, 255].
	------------------------------------------------*/
	struct Color
	{
		unsigned char red;
		unsigned char green;
		unsigned char blue;

		/* Components are in order RGB and in range [0, 1] */
		explicit operator Vector3() const;
	};
}