#include "settings.h"

#include <utility>

namespace leopph
{
	std::filesystem::path Settings::s_ShaderLocation{};
	const Settings::GraphicsAPI Settings::RenderAPI{ GraphicsAPI::OpenGL };
	Settings::GraphicsAPI Settings::s_PendingRenderAPI{ RenderAPI };

	bool Settings::IsCachingShaders()
	{
		return !s_ShaderLocation.empty();
	}

	const std::filesystem::path& Settings::ShaderCacheLocation()
	{
		return s_ShaderLocation;
	}

	void Settings::ShaderCacheLocation(std::filesystem::path path)
	{
		s_ShaderLocation = std::move(path);
	}

	void Settings::SetRenderAPI(GraphicsAPI newAPI)
	{
		s_PendingRenderAPI = newAPI;
	}
}