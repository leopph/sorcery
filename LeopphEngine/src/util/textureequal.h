#pragma once

#include "texturereference.h"

#include <filesystem>

namespace leopph::impl
{
	struct TextureEqual
	{
		using is_transparent = void;

		bool operator()(const TextureReference& left, const TextureReference& right) const;
		bool operator()(const TextureReference& left, const decltype(TextureReference::id)& right) const;
		bool operator()(const decltype(TextureReference::id)& left, const TextureReference& right) const;
		bool operator()(const TextureReference& left, const std::filesystem::path& right) const;
		bool operator()(const std::filesystem::path& left, const TextureReference& right) const;
	};
}