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

	auto Settings::CacheShaders() -> bool
	{
		return !s_ShaderLocation.empty();
	}

	auto Settings::ShaderCacheLocation() -> const std::filesystem::path&
	{
		return s_ShaderLocation;
	}

	auto Settings::ShaderCacheLocation(std::filesystem::path path) -> void
	{
		s_ShaderLocation = std::move(path);
	}

	auto Settings::SetRenderApi(const GraphicsApi newApi) -> void
	{
		s_PendingRenderApi = newApi;
	}

	LEOPPHAPI auto Settings::Vsync() -> bool
	{
		return internal::WindowBase::Get().Vsync();
	}

	LEOPPHAPI auto Settings::Vsync(const bool value) -> void
	{
		internal::WindowBase::Get().Vsync(value);
	}

	auto Settings::DirectionalShadowMapResolutions() -> const std::vector<std::size_t>&
	{
		return s_DirectionalLightShadowMapResolutions;
	}

	auto Settings::DirectionalShadowMapResolutions(std::vector<std::size_t> newRess) -> void
	{
		s_DirectionalLightShadowMapResolutions = std::move(newRess);
		EventManager::Instance().Send<internal::DirShadowResolutionEvent>(s_DirectionalLightShadowMapResolutions);
	}

	auto Settings::DirectionalShadowCascadeCount() -> std::size_t
	{
		return s_DirectionalLightShadowMapResolutions.size();
	}

	auto Settings::PointLightShadowMapResolution() -> std::size_t
	{
		return s_PointLightShadowMapResolution;
	}

	auto Settings::PointLightShadowMapResolution(const std::size_t newRes) -> void
	{
		s_PointLightShadowMapResolution = newRes;
	}

	auto Settings::SpotLightShadowMapResolution() -> std::size_t
	{
		return s_SpotLightShadowMapResolution;
	}

	auto Settings::SpotLightShadowMapResolution(const std::size_t newRes) -> void
	{
		s_SpotLightShadowMapResolution = newRes;
		EventManager::Instance().Send<internal::SpotShadowResolutionEvent>(s_SpotLightShadowMapResolution);
	}

	auto Settings::MaxPointLightCount() -> std::size_t
	{
		return s_MaxPointLightCount;
	}

	auto Settings::MaxPointLightCount(const std::size_t newCount) -> void
	{
		s_MaxPointLightCount = newCount;
	}

	auto Settings::MaxSpotLightCount() -> std::size_t
	{
		return s_MaxSpotLightCount;
	}

	auto Settings::MaxSpotLightCount(const std::size_t newCount) -> void
	{
		s_MaxSpotLightCount = newCount;
	}

	auto Settings::RenderingPipeline() -> Settings::RenderType
	{
		return s_RenderingPipeline;
	}

	auto Settings::RenderingPipeline(const RenderType type) -> void
	{
		s_RenderingPipeline = type;
	}
}
