#include "PoeloBaseLess.hpp"


namespace leopph::internal
{
	auto PoeloBaseLess::operator()(const std::unique_ptr<PoeloBase>& left, const std::unique_ptr<PoeloBase>& right) const -> bool
	{
		return left < right;
	}


	auto PoeloBaseLess::operator()(const std::unique_ptr<PoeloBase>& left, const PoeloBase* right) const -> bool
	{
		return left.get() < right;
	}


	auto PoeloBaseLess::operator()(const PoeloBase* left, const std::unique_ptr<PoeloBase>& right) const -> bool
	{
		return left < right.get();
	}
}
