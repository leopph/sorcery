#include "PathHash.hpp"

auto leopph::internal::PathHash::operator()(const std::filesystem::path& path) const -> std::size_t
{
	return std::filesystem::hash_value(path);
}
