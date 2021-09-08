#include "Settings.hpp"

#include "../windowing/window.h"
#include "../events/DirShadowMapResChangedEvent.hpp"
#include "../events/EventManager.hpp"

#include <utility>

namespace leopph
{
	std::filesystem::path Settings::s_ShaderLocation{};
	const Settings::GraphicsAPI Settings::RenderAPI{ GraphicsAPI::OpenGL };
	Settings::GraphicsAPI Settings::s_PendingRenderAPI{ RenderAPI };
	std::vector<std::size_t> Settings::s_DirectionalLightShadowMapResolutions{4096, 3072, 2048};
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


	const std::vector<std::size_t>& Settings::DirectionalShadowMapResolutions()
	{
		return s_DirectionalLightShadowMapResolutions;
	}


	void Settings::DirectionalShadowMapResolutions(std::vector<std::size_t> newRess)
	{
		s_DirectionalLightShadowMapResolutions = std::move(newRess);
		EventManager::Instance().Send<impl::DirShadowMapResChangedEvent>(s_DirectionalLightShadowMapResolutions);
	}


	std::size_t Settings::CameraDirectionalShadowCascadeCount()
	{
		return s_DirectionalLightShadowMapResolutions.size();
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