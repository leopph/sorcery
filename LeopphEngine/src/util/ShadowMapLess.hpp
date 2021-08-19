#pragma once

#include "../rendering/ShadowMap.hpp"

namespace std
{
	template<>
	struct less<leopph::impl::ShadowMap>
	{
		bool operator()(const leopph::impl::ShadowMap& left, const leopph::impl::ShadowMap& right) const;
	};
}