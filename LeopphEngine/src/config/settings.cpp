#include "Settings.hpp"

#include "../events/DirCascadeChangeEvent.hpp"
#include "../events/SpotShadowResolutionEvent.hpp"
#include "../events/handling/EventManager.hpp"
#include "../windowing/WindowBase.hpp"


namespace leopph
{
	std::filesystem::path Settings::s_CacheLoc;
	std::vector<ShadowCascade> Settings::s_DirShadowRes{{4096, 15}, {2048, 50}, {1024, 200}};
	std::size_t Settings::s_SpotShadowRes{2048};
	std::size_t Settings::s_PointShadowRes{1024};
	std::size_t Settings::s_NumMaxSpot{64};
	std::size_t Settings::s_NumMaxPoint{64};
	Settings::GraphicsApi Settings::s_Api{GraphicsApi::OpenGl};
	Settings::GraphicsApi Settings::s_PendingApi{GraphicsApi::OpenGl};
	Settings::RenderType Settings::s_Pipeline{RenderType::Deferred};
	Settings::RenderType Settings::s_PendingPipeline{RenderType::Deferred};


	auto Settings::CacheShaders() -> bool
	{
		return !s_CacheLoc.empty();
	}


	LEOPPHAPI auto Settings::Vsync() -> bool
	{
		return internal::WindowBase::Get().Vsync();
	}


	LEOPPHAPI auto Settings::Vsync(const bool value) -> void
	{
		internal::WindowBase::Get().Vsync(value);
	}


	auto Settings::DirShadowCascades(std::span<const ShadowCascade> cascades) -> void
	{
		s_DirShadowRes.assign(cascades.begin(), cascades.end());
		EventManager::Instance().Send<internal::DirCascadeChangeEvent>(s_DirShadowRes);
	}


	auto Settings::DirShadowCascadeCount() -> std::size_t
	{
		return s_DirShadowRes.size();
	}


	auto Settings::SpotLightShadowMapResolution(const std::size_t newRes) -> void
	{
		s_SpotShadowRes = newRes;
		EventManager::Instance().Send<internal::SpotShadowResolutionEvent>(s_SpotShadowRes);
	}
}
