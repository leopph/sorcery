#include "Settings.hpp"

#include "EventManager.hpp"
#include "InternalContext.hpp"
#include "events/DirShadowResEvent.hpp"
#include "events/PointShadowResEvent.hpp"
#include "events/SpotShadowResEvent.hpp"
#include "windowing/WindowImpl.hpp"

// ReSharper disable All
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Psapi.h>
// ReSharper restore All


namespace leopph
{
	std::filesystem::path const& Settings::get_shader_cache_path()
	{
		return m_ShaderCache;
	}



	void Settings::set_shader_cache_path(std::filesystem::path path)
	{
		m_ShaderCache = std::move(path);
		m_Serialize = true;
	}



	bool Settings::is_caching_shaders() const
	{
		return !m_ShaderCache.empty();
	}



	Settings::GraphicsApi Settings::get_current_graphics_api() const
	{
		return m_RenderingSettings.Api;
	}



	void Settings::set_graphics_api(GraphicsApi const graphicsApi)
	{
		m_RenderingSettings.PendingApi = graphicsApi;
		m_Serialize = true;
	}



	Settings::GraphicsPipeline Settings::GetGraphicsPipeline() const noexcept
	{
		return m_RenderingSettings.Pipe;
	}



	void Settings::SetGraphicsPipeline(GraphicsPipeline const pipeline) noexcept
	{
		m_RenderingSettings.PendingPipe = pipeline;
		m_Serialize = true;
	}



	std::span<u16 const> Settings::DirShadowResolution()
	{
		return m_DirLightSettings.Res;
	}



	void Settings::DirShadowResolution(std::span<u16 const> cascades)
	{
		// Windows.h bullshit
		#undef max

		if (auto constexpr maxCascades = std::numeric_limits<u8>::max(); cascades.size() > maxCascades)
		{
			cascades = cascades.subspan(0, maxCascades);
		}
		m_DirLightSettings.Res.assign(cascades.begin(), cascades.end());
		m_Serialize = true;
		EventManager::Instance().Send<internal::DirShadowResEvent>();
	}



	f32 Settings::DirShadowCascadeCorrection() const noexcept
	{
		return m_DirLightSettings.Corr;
	}



	void Settings::DirShadowCascadeCorrection(f32 const newCor) noexcept
	{
		m_DirLightSettings.Corr = newCor;
		m_Serialize = true;
	}



	u8 Settings::DirShadowCascadeCount() const noexcept
	{
		return static_cast<u8>(m_DirLightSettings.Res.size());
	}



	u16 Settings::SpotShadowResolution() const noexcept
	{
		return m_SpotLightSettings.Res;
	}



	void Settings::SpotShadowResolution(u16 const newRes)
	{
		m_SpotLightSettings.Res = newRes;
		m_Serialize = true;
		EventManager::Instance().Send<internal::SpotShadowResEvent>();
	}



	u8 Settings::MaxSpotLightCount() const noexcept
	{
		return m_SpotLightSettings.MaxNum;
	}



	void Settings::MaxSpotLightCount(u8 const newCount) noexcept
	{
		m_SpotLightSettings.MaxNum = newCount;
		m_Serialize = true;
	}



	u16 Settings::PointShadowResolution() const noexcept
	{
		return m_PointLightSettings.Res;
	}



	void Settings::PointShadowResolution(u16 const newRes) noexcept
	{
		m_PointLightSettings.Res = newRes;
		m_Serialize = true;
		EventManager::Instance().Send<internal::PointShadowResEvent>();
	}



	u8 Settings::MaxPointLightCount() const noexcept
	{
		return m_PointLightSettings.MaxNum;
	}



	auto Settings::MaxPointLightCount(u8 const newCount) noexcept
	{
		m_PointLightSettings.MaxNum = newCount;
		m_Serialize = true;
	}



	u32 Settings::WindowWidth() const noexcept
	{
		return m_WindowSettingsCache.Width;
	}



	void Settings::WindowWidth(u32 const newWidth) noexcept
	{
		m_WindowSettingsCache.Width = newWidth;
		internal::GetWindowImpl()->set_width(newWidth);
		m_Serialize = true;
	}



	u32 Settings::WindowHeight() const noexcept
	{
		return m_WindowSettingsCache.Height;
	}



	void Settings::WindowHeight(u32 const newHeight) noexcept
	{
		m_WindowSettingsCache.Height = newHeight;
		internal::GetWindowImpl()->set_height(newHeight);
		m_Serialize = true;
	}



	f32 Settings::RenderMultiplier() const noexcept
	{
		return m_WindowSettingsCache.RenderMultiplier;
	}



	void Settings::RenderMultiplier(f32 const newMult) noexcept
	{
		m_WindowSettingsCache.RenderMultiplier = newMult;
		internal::GetWindowImpl()->set_render_multiplier(newMult);
		m_Serialize = true;
	}



	bool Settings::Fullscreen() const noexcept
	{
		return m_WindowSettingsCache.Fullscreen;
	}



	void Settings::Fullscreen(bool const newVal) noexcept
	{
		m_WindowSettingsCache.Fullscreen = newVal;
		internal::GetWindowImpl()->set_fullscreen(newVal);
		m_Serialize = true;
	}



	bool Settings::Vsync() const
	{
		return m_WindowSettingsCache.Vsync;
	}



	void Settings::Vsync(bool const newVal)
	{
		m_WindowSettingsCache.Vsync = newVal;
		internal::GetWindowImpl()->set_vsync(newVal);
		m_Serialize = true;
	}



	f32 Settings::Gamma() const
	{
		return m_RenderingSettings.Gamma;
	}



	void Settings::Gamma(f32 const newVal)
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
