#include "PathHash.hpp"

std::size_t leopph::impl::PathHash::operator()(const std::filesystem::path& path) const
{
	return std::filesystem::hash_value(path);
}