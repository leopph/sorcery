#pragma once

#include "../misc/skybox.h"
#include "../instances/skyboximpl.h"

#include <string>

namespace leopph::impl
{
	struct SkyboxImplEqual
	{
		using is_transparent = void;

		bool operator()(const SkyboxImpl& left, const SkyboxImpl& right) const;
		bool operator()(const std::string& left, const SkyboxImpl& right) const;
		bool operator()(const SkyboxImpl& left, const std::string& right) const;
		bool operator()(const SkyboxImpl& left, const Skybox& right) const;
		bool operator()(const Skybox& left, const SkyboxImpl& right) const;
	};
}