#pragma once

#include "Color.hpp"
#include "Texture.hpp"
#include "Types.hpp"

#include <memory>


namespace leopph
{
	struct Material
	{
		// RGB controls the diffuse color, A controls the opacity.
		// For opaque surfaces, the opacity only takes part in alpha clipping,
		// while for transparent ones, it also affects blending.
		Color diffuseColor{255, 255, 255, 255};

		// RGB controls the color of the specular highlights, A is unused.
		Color specularColor{0, 0, 0};

		// Controls the spread of the specular highlight.
		f32 gloss{0};

		// RGB is multiplied with diffuseColor RGB, A is multiplied with the color alpha and used accordingly, if present.
		std::shared_ptr<Texture> diffuseMap;

		// RGB is multiplied with specularColor RGB, A is unused if present.
		std::shared_ptr<Texture> specularMap;

		// Discards pixels whose alpha values are under this value.
		f32 alphaThreshold{1.f};

		bool cullBackFace{true};
		bool isTransparent{false};
	};
}