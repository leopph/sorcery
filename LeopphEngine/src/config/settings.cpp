#include "Settings.hpp"

#include "../events/DirShadowResolutionEvent.hpp"
#include "../events/SpotShadowResolutionEvent.hpp"
#include "../events/handling/EventManager.hpp"
#include "../windowing/WindowBase.hpp"

#include <utility>



namespace leopph
{
	std::filesystem::path Settings::s_ShaderLocation{};
	const Settings::GraphicsApi Settings::RenderAPI{GraphicsApi::OpenGl};
	Settings::GraphicsApi Settings::s_PendingRenderApi{RenderAPI};
	std::vector<std::size_t> Settings::s_DirectionalLightShadowMapResolutions{4096, 2048, 1024};
	std::size_t Settings::s_PointLightShadowMapResolution{1024};
	std::size_t Settings::s_SpotLightShadowMapResolution{2048};
	std::size_t Settings::s_MaxPointLightCount{64};
	std::size_t Settings::s_MaxSpotLightCount{64};
	Settings::RenderType Settings::s_RenderingPipeline{RenderType::Deferred};


	bool Settings::CacheShaders()
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


	void Settings::SetRenderApi(const GraphicsApi newApi)
	{
		s_PendingRenderApi = newApi;
	}


	LEOPPHAPI bool Settings::Vsync()
	{
		return impl::WindowBase::Get().Vsync();
	}


	LEOPPHAPI void Settings::Vsync(const bool value)
	{
		impl::WindowBase::Get().Vsync(value);
	}


	const std::vector<std::size_t>& Settings::DirectionalShadowMapResolutions()
	{
		return s_DirectionalLightShadowMapResolutions;
	}


	void Settings::DirectionalShadowMapResolutions(std::vector<std::size_t> newRess)
	{
		s_DirectionalLightShadowMapResolutions = std::move(newRess);
		EventManager::Instance().Send<impl::DirShadowResolutionEvent>(s_DirectionalLightShadowMapResolutions);
	}


	std::size_t Settings::DirectionalShadowCascadeCount()
	{
		return s_DirectionalLightShadowMapResolutions.size();
	}


	std::size_t Settings::PointLightShadowMapResolution()
	{
		return s_PointLightShadowMapResolution;
	}


	void Settings::PointLightShadowMapResolution(const std::size_t newRes)
	{
		s_PointLightShadowMapResolution = newRes;
	}


	std::size_t Settings::SpotLightShadowMapResolution()
	{
		return s_SpotLightShadowMapResolution;
	}


	void Settings::SpotLightShadowMapResolution(const std::size_t newRes)
	{
		s_SpotLightShadowMapResolution = newRes;
		EventManager::Instance().Send<impl::SpotShadowResolutionEvent>(s_SpotLightShadowMapResolution);
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


	Settings::RenderType Settings::RenderingPipeline()
	{
		return s_RenderingPipeline;
	}


	void Settings::RenderingPipeline(const RenderType type)
	{
		s_RenderingPipeline = type;
	}
}
