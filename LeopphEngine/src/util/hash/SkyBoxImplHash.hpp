#pragma once

#include "../../misc/skybox.h"
#include "../../instances/skyboximpl.h"

#include <cstddef>
#include <string>

namespace leopph::impl
{
	struct SkyboxImplHash
	{
		using is_transparent = void;

		std::size_t operator()(const SkyboxImpl& skyboxImpl) const;
		std::size_t operator()(const std::string& string) const;
		std::size_t operator()(const Skybox& skybox) const;
	};
}