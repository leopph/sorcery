#include "util/less/PoeloLess.hpp"


namespace leopph::internal
{
	auto PoeloLess::operator()(const std::unique_ptr<Poelo>& left, const std::unique_ptr<Poelo>& right) const -> bool
	{
		return left < right;
	}


	auto PoeloLess::operator()(const std::unique_ptr<Poelo>& left, const Poelo* right) const -> bool
	{
		return left.get() < right;
	}


	auto PoeloLess::operator()(const Poelo* left, const std::unique_ptr<Poelo>& right) const -> bool
	{
		return left < right.get();
	}
}
