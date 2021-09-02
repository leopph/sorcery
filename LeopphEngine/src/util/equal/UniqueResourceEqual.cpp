#include "UniqueResourceEqual.hpp"


namespace leopph::impl
{
	bool UniqueResourceEqual::operator()(const UniqueResource* left, const UniqueResource* right) const
	{
		return left->Path == right->Path;
	}


	bool UniqueResourceEqual::operator()(const UniqueResource* left, const std::filesystem::path& right) const
	{
		return left->Path == right;
	}


	bool UniqueResourceEqual::operator()(const std::filesystem::path& left, const UniqueResource* right) const
	{
		return left == right->Path;
	}
}
