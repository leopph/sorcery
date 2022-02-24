#include "Settings.hpp"

#include "../events/DirShadowResChangeEvent.hpp"
#include "../events/SpotShadowResolutionEvent.hpp"
#include "../events/handling/EventManager.hpp"
#include "../windowing/WindowBase.hpp"


namespace leopph
{
	auto Settings::Instance() -> Settings&
	{
		static Settings instance;
		return instance;
	}


	auto Settings::CacheShaders() const -> bool
	{
		return !m_CacheLoc.empty();
	}


	LEOPPHAPI auto Settings::Vsync() const -> bool
	{
		return internal::WindowBase::Get().Vsync();
	}


	LEOPPHAPI auto Settings::Vsync(const bool value) const -> void
	{
		internal::WindowBase::Get().Vsync(value);
	}


	auto Settings::DirShadowRes(std::span<const std::size_t> cascades) -> void
	{
		m_DirShadowRes.assign(cascades.begin(), cascades.end());
		EventManager::Instance().Send<internal::DirShadowResChangeEvent>(m_DirShadowRes);
	}


	auto Settings::DirShadowCascadeCount() const -> std::size_t
	{
		return m_DirShadowRes.size();
	}


	auto Settings::SpotLightShadowMapResolution(const std::size_t newRes) -> void
	{
		m_SpotShadowRes = newRes;
		EventManager::Instance().Send<internal::SpotShadowResolutionEvent>(m_SpotShadowRes);
	}
}
