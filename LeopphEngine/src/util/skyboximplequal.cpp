#include "skyboximplequal.h"

bool leopph::impl::SkyboxImplEqual::operator()(const SkyboxImpl& left, const SkyboxImpl& right) const
{
	return left.fileNames == right.fileNames;
}

bool leopph::impl::SkyboxImplEqual::operator()(const Skybox& left, const SkyboxImpl& right) const
{
	return left == right;
}

bool leopph::impl::SkyboxImplEqual::operator()(const SkyboxImpl& left, const Skybox& right) const
{
	return left == right;
}

bool leopph::impl::SkyboxImplEqual::operator()(const std::string& left, const SkyboxImpl& right) const
{
	return left == right.fileNames;
}

bool leopph::impl::SkyboxImplEqual::operator()(const SkyboxImpl& left, const std::string& right) const
{
	return left.fileNames == right;
}