#include "Settings.hpp"

#include "../windowing/window.h"

#include <utility>

namespace leopph
{
	std::filesystem::path Settings::s_ShaderLocation{};
	const Settings::GraphicsAPI Settings::RenderAPI{ GraphicsAPI::OpenGL };
	Settings::GraphicsAPI Settings::s_PendingRenderAPI{ RenderAPI };
	Vector2 Settings::s_DirectionalLightShadowMapResolution{ 4096, 4096 };
	Vector2 Settings::s_PointLightShadowMapResolution{ 1024, 1024 };
	Vector2 Settings::s_SpotLightShadowMapResolution{ 2048, 2048 };
	std::size_t Settings::s_MaxPointLightCount{ 64 };
	std::size_t Settings::s_MaxSpotLightCount{ 64 };

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

	void Settings::SetRenderAPI(const GraphicsAPI newAPI)
	{
		s_PendingRenderAPI = newAPI;
	}

	LEOPPHAPI bool Settings::Vsync()
	{
		return impl::Window::Get().Vsync();
	}

	LEOPPHAPI void Settings::Vsync(const bool value)
	{
		impl::Window::Get().Vsync(value);
	}

	const Vector2& Settings::DirectionalLightShadowMapResolution()
	{
		return s_DirectionalLightShadowMapResolution;
	}

	void Settings::DirectionalLightShadowMapResolution(const Vector2& newRes)
	{
		s_DirectionalLightShadowMapResolution = newRes;
	}

	const Vector2& Settings::PointLightShadowMapResolution()
	{
		return s_PointLightShadowMapResolution;
	}

	void Settings::PointLightShadowMapResolution(const Vector2& newRes)
	{
		s_PointLightShadowMapResolution = newRes;
	}

	const Vector2& Settings::SpotLightShadowMapResolution()
	{
		return s_SpotLightShadowMapResolution;
	}

	void Settings::SpotLightShadowMapResolution(const Vector2& newRes)
	{
		s_SpotLightShadowMapResolution = newRes;
	}

	std::size_t Settings::MaxPointLightCount()
	{
		return s_MaxPointLightCount;
	}

	void Settings::MaxPointLightCount(const std::size_t newCount)
	{
		s_MaxPointLightCount = newCount;
	}

	std::size_t Settings::MaxSpotLightCount()
	{
		return s_MaxSpotLightCount;
	}

	void Settings::MaxSpotLightCount(const std::size_t newCount)
	{
		s_MaxSpotLightCount = newCount;
	}

}