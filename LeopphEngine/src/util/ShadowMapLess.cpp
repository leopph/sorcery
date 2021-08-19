#include "ShadowMapLess.hpp"

using leopph::impl::ShadowMap;

namespace std
{
	bool less<ShadowMap>::operator()(const ShadowMap& left, const ShadowMap& right) const
	{
		return left.id < right.id;
	}

}