#include "PoeloLess.hpp"


namespace leopph::internal
{
	bool PoeloLess::operator()(std::unique_ptr<Poelo> const& left, std::unique_ptr<Poelo> const& right) const
	{
		return left < right;
	}


	bool PoeloLess::operator()(std::unique_ptr<Poelo> const& left, Poelo const* right) const
	{
		return left.get() < right;
	}


	bool PoeloLess::operator()(Poelo const* left, std::unique_ptr<Poelo> const& right) const
	{
		return left < right.get();
	}
}
