#include "UniqueResourceHash.hpp"


namespace leopph::impl
{
	std::size_t UniqueResourceHash::operator()(const UniqueResource* resource) const
	{
		return hash_value(resource->Path);
	}


	std::size_t UniqueResourceHash::operator()(const std::filesystem::path& path) const
	{
		return hash_value(path);
	}
}
