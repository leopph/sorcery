#include "PathHash.hpp"

std::size_t leopph::internal::PathHash::operator()(const std::filesystem::path& path) const
{
	return std::filesystem::hash_value(path);
}