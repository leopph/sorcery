#pragma once

#include "../../data/Poelo.hpp"

#include <memory>


namespace leopph::internal
{
	struct PoeloLess
	{
		using is_transparent = void;

		auto operator()(const std::unique_ptr<Poelo>& left, const std::unique_ptr<Poelo>& right) const -> bool;
		auto operator()(const std::unique_ptr<Poelo>& left, const Poelo* right) const -> bool;
		auto operator()(const Poelo* left, const std::unique_ptr<Poelo>& right) const -> bool;
	};
}
