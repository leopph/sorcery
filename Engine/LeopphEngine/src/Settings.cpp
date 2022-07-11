#include "Settings.hpp"

#include "EventManager.hpp"
#include "InternalContext.hpp"
#include "events/DirShadowEvent.hpp"
#include "events/PointShadowEvent.hpp"
#include "events/SpotShadowEvent.hpp"
#include "windowing/WindowImpl.hpp"

// ReSharper disable All
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Psapi.h>
// ReSharper restore All


namespace leopph
{
	auto Settings::ShaderCachePath() noexcept -> std::filesystem::path const&
	{
		return m_ShaderCache;
	}



	auto Settings::ShaderCachePath(std::filesystem::path path) noexcept -> void
	{
		m_ShaderCache = std::move(path);
		m_Serialize = true;
	}



	auto Settings::CacheShaders() const -> bool
	{
		return !m_ShaderCache.empty();
	}



	auto Settings::GetGraphicsApi() const noexcept -> GraphicsApi
	{
		return m_RenderingSettings.Api;
	}



	auto Settings::SetGraphicsApi(GraphicsApi const newApi) noexcept -> void
	{
		m_RenderingSettings.PendingApi = newApi;
		m_Serialize = true;
	}



	auto Settings::GetGraphicsPipeline() const noexcept -> GraphicsPipeline
	{
		return m_RenderingSettings.Pipe;
	}



	auto Settings::SetGraphicsPipeline(GraphicsPipeline const pipeline) noexcept -> void
	{
		m_RenderingSettings.PendingPipe = pipeline;
		m_Serialize = true;
	}



	auto Settings::DirShadowResolution() -> std::span<u64 const>
	{
		return m_DirLightSettings.Res;
	}



	auto Settings::DirShadowResolution(std::span<std::size_t const> cascades) -> void
	{
		m_DirLightSettings.Res.assign(cascades.begin(), cascades.end());
		m_Serialize = true;
		EventManager::Instance().Send<internal::DirShadowEvent>(m_DirLightSettings.Res);
	}



	auto Settings::DirShadowCascadeCorrection() const noexcept -> f32
	{
		return m_DirLightSettings.Corr;
	}



	auto Settings::DirShadowCascadeCorrection(f32 const newCor) noexcept -> void
	{
		m_DirLightSettings.Corr = newCor;
		m_Serialize = true;
	}



	auto Settings::DirShadowCascadeCount() const noexcept -> u64
	{
		return m_DirLightSettings.Res.size();
	}



	auto Settings::SpotShadowResolution() const noexcept -> u64
	{
		return m_SpotLightSettings.Res;
	}



	auto Settings::SpotShadowResolution(u64 const newRes) -> void
	{
		m_SpotLightSettings.Res = newRes;
		m_Serialize = true;
		EventManager::Instance().Send<internal::SpotShadowEvent>(m_SpotLightSettings.Res);
	}



	auto Settings::MaxSpotLightCount() const noexcept -> u64
	{
		return m_SpotLightSettings.MaxNum;
	}



	auto Settings::MaxSpotLightCount(u64 const newCount) noexcept -> void
	{
		m_SpotLightSettings.MaxNum = newCount;
		m_Serialize = true;
	}



	auto Settings::PointShadowResolution() const noexcept -> u64
	{
		return m_PointLightSettings.Res;
	}



	auto Settings::PointShadowResolution(u64 const newRes) noexcept -> void
	{
		m_PointLightSettings.Res = newRes;
		m_Serialize = true;
		EventManager::Instance().Send<internal::PointShadowEvent>(m_PointLightSettings.Res);
	}



	auto Settings::MaxPointLightCount() const noexcept -> u64
	{
		return m_PointLightSettings.MaxNum;
	}



	auto Settings::MaxPointLightCount(u64 const newCount) noexcept
	{
		m_PointLightSettings.MaxNum = newCount;
		m_Serialize = true;
	}



	auto Settings::WindowWidth() const noexcept -> u32
	{
		return m_WindowSettingsCache.Width;
	}



	auto Settings::WindowWidth(u32 const newWidth) noexcept -> void
	{
		m_WindowSettingsCache.Width = newWidth;
		internal::GetWindowImpl()->Width(newWidth);
		m_Serialize = true;
	}



	auto Settings::WindowHeight() const noexcept -> u32
	{
		return m_WindowSettingsCache.Height;
	}



	auto Settings::WindowHeight(u32 const newHeight) noexcept -> void
	{
		m_WindowSettingsCache.Height = newHeight;
		internal::GetWindowImpl()->Height(newHeight);
		m_Serialize = true;
	}



	auto Settings::RenderMultiplier() const noexcept -> f32
	{
		return m_WindowSettingsCache.RenderMultiplier;
	}



	auto Settings::RenderMultiplier(f32 const newMult) noexcept -> void
	{
		m_WindowSettingsCache.RenderMultiplier = newMult;
		internal::GetWindowImpl()->RenderMultiplier(newMult);
		m_Serialize = true;
	}



	auto Settings::Fullscreen() const noexcept -> bool
	{
		return m_WindowSettingsCache.Fullscreen;
	}



	auto Settings::Fullscreen(bool const newVal) noexcept -> void
	{
		m_WindowSettingsCache.Fullscreen = newVal;
		internal::GetWindowImpl()->Fullscreen(newVal);
		m_Serialize = true;
	}



	auto Settings::Vsync() const -> bool
	{
		return m_WindowSettingsCache.Vsync;
	}



	auto Settings::Vsync(bool const newVal) -> void
	{
		m_WindowSettingsCache.Vsync = newVal;
		internal::GetWindowImpl()->Vsync(newVal);
		m_Serialize = true;
	}



	auto Settings::Gamma() const -> f32
	{
		return m_RenderingSettings.Gamma;
	}



	auto Settings::Gamma(f32 const newVal) -> void
	{
		m_RenderingSettings.Gamma = newVal;
	}



	std::filesystem::path Settings::s_FilePath
	{
		[]
		{
			constexpr auto bufSz{100u};
			constexpr auto defChar{'\0'};
			#ifdef UNICODE
			std::wstring s(bufSz, defChar);
			#else
			std::string s(bufSz, defChar);
			#endif
			s.resize(GetModuleFileNameEx(GetCurrentProcess(), nullptr, s.data(), static_cast<DWORD>(s.size())));
			return std::filesystem::path(s).parent_path() / "settings.json";
		}()
	};
}
