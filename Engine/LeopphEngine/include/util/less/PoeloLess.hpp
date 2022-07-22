#pragma once

#include "Poelo.hpp"

#include <memory>


namespace leopph::internal
{
	struct PoeloLess
	{
		using is_transparent = void;

		bool operator()(std::unique_ptr<Poelo> const& left, std::unique_ptr<Poelo> const& right) const;
		bool operator()(std::unique_ptr<Poelo> const& left, Poelo const* right) const;
		bool operator()(Poelo const* left, std::unique_ptr<Poelo> const& right) const;
	};
}
