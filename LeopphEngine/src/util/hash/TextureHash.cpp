#include "TextureHash.hpp"

#include <filesystem>
#include <functional>
#include <type_traits>

std::size_t leopph::impl::TextureHash::operator()(const TextureReference& texture) const
{
	return std::filesystem::hash_value(texture.path);
}

std::size_t leopph::impl::TextureHash::operator()(const std::filesystem::path& path) const
{
	return std::filesystem::hash_value(path);
}

std::size_t leopph::impl::TextureHash::operator()(const decltype(TextureReference::id)& id) const
{
	return std::hash<std::decay<decltype(id)>::type>{}(id);
}