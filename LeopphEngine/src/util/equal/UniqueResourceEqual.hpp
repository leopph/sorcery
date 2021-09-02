#pragma once

#include <filesystem>

#include "../../data/managed/UniqueResource.hpp"


namespace leopph::impl
{
	struct UniqueResourceEqual
	{
		using is_transparent = void;

		bool operator()(const UniqueResource* left, const UniqueResource* right) const;
		bool operator()(const std::filesystem::path& left, const UniqueResource* right) const;
		bool operator()(const UniqueResource* left, const std::filesystem::path& right) const;
	};
}