#include "Color.hpp"

#include <algorithm>


namespace leopph
{
	Color::Color(Vector3 const& vec) noexcept :
		Red{static_cast<unsigned char>(std::clamp(vec[0], 0.f, 1.f) * 255)},
		Green{static_cast<unsigned char>(std::clamp(vec[1], 0.f, 1.f) * 255)},
		Blue{static_cast<unsigned char>(std::clamp(vec[2], 0.f, 1.f) * 255)}
	{}


	Color::operator Vector3() const
	{
		return Vector3{static_cast<float>(Red) / 255.f, static_cast<float>(Green) / 255.f, static_cast<float>(Blue) / 255.f};
	}
}
