#include "SkyBoxImplHash.hpp"

std::size_t leopph::impl::SkyboxImplHash::operator()(const SkyboxImpl& skyboxImpl) const
{
	return std::hash<std::string>{}(skyboxImpl.fileNames);
}

std::size_t leopph::impl::SkyboxImplHash::operator()(const Skybox& skybox) const
{
	return std::hash<std::string>{}(skybox.FileNames());
}

std::size_t leopph::impl::SkyboxImplHash::operator()(const std::string& string) const
{
	return std::hash<std::string>{}(string);
}