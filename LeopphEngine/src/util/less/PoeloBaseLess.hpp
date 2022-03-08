#pragma once

#include "../../data/PoeloBase.hpp"

#include <memory>


namespace leopph::internal
{
	struct PoeloBaseLess
	{
		using is_transparent = void;

		auto operator()(const std::unique_ptr<PoeloBase>& left, const std::unique_ptr<PoeloBase>& right) const -> bool;
		auto operator()(const std::unique_ptr<PoeloBase>& left, const PoeloBase* right) const -> bool;
		auto operator()(const PoeloBase* left, const std::unique_ptr<PoeloBase>& right) const -> bool;
	};
}
