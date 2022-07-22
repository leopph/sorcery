#pragma once

#include "LeopphApi.hpp"
#include "Vector.hpp"


namespace leopph
{
	// Represents RGB colors.
	// Components are in the range [0, 255].
	struct Color
	{
		unsigned char Red{255};
		unsigned char Green{255};
		unsigned char Blue{255};

		constexpr Color(unsigned char red, unsigned char green, unsigned char blue) noexcept;

		// Components must be in [0, 1].
		// Values outside [0, 1] will be clamped.
		// They are interpreted as RGB values and are mapped to [0, 255].
		LEOPPHAPI explicit Color(Vector3 const& vec) noexcept;

		// Components are in RGB order and mapped to [0, 1].
		LEOPPHAPI explicit operator Vector3() const;
	};


	constexpr Color::Color(unsigned char const red, unsigned char const green, unsigned char const blue) noexcept :
		Red{red},
		Green{green},
		Blue{blue}
	{}
}
