#include "Settings.hpp"

#include "EventManager.hpp"
#include "Logger.hpp"
#include "event/DirShadowEvent.hpp"
#include "event/PointShadowEvent.hpp"
#include "event/SpotShadowEvent.hpp"
#include "windowing/WindowImpl.hpp"

#include <exception>
#include <fstream>
#include <json.hpp>

// ReSharper disable All
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Psapi.h>
// ReSharper restore All

namespace leopph
{
	std::filesystem::path Settings::s_FilePath{
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


	Settings::Settings()
	{
		if (std::filesystem::exists(s_FilePath))
		{
			// If we already have a stored configuration, we parse that one
			Deserialize();
		}
		else
		{
			// Otherwise we defer serialization to the end of the first frame
			// so that we know for sure that all to-be-serialized objects exist and are initialized.
			m_Serialize = true;
		}
	}


	auto Settings::Instance() -> Settings&
	{
		static Settings instance;
		return instance;
	}


	auto Settings::CacheShaders() const -> bool
	{
		return !m_ShaderCache.empty();
	}


	auto Settings::DirShadowResolution(std::span<std::size_t const> cascades) -> void
	{
		m_DirLightSettings.Res.assign(cascades.begin(), cascades.end());
		m_Serialize = true;
		EventManager::Instance().Send<internal::DirShadowEvent>(m_DirLightSettings.Res);
	}


	auto Settings::SpotShadowResolution(std::size_t const newRes) -> void
	{
		m_SpotLightSettings.Res = newRes;
		m_Serialize = true;
		EventManager::Instance().Send<internal::SpotShadowEvent>(m_SpotLightSettings.Res);
	}


	auto Settings::PointShadowResolution(std::size_t const newRes) noexcept -> void
	{
		m_PointLightSettings.Res = newRes;
		m_Serialize = true;
		EventManager::Instance().Send<internal::PointShadowEvent>(m_PointLightSettings.Res);
	}


	auto Settings::WindowWidth() const noexcept -> unsigned
	{
		return m_WindowSettingsCache.Width;
	}


	auto Settings::WindowWidth(unsigned const newWidth) noexcept -> void
	{
		m_WindowSettingsCache.Width = newWidth;
		Window::Instance()->Width(newWidth);
		m_Serialize = true;
	}


	auto Settings::WindowHeight() const noexcept -> unsigned
	{
		return m_WindowSettingsCache.Height;
	}


	auto Settings::WindowHeight(unsigned const newHeight) noexcept -> void
	{
		m_WindowSettingsCache.Height = newHeight;
		Window::Instance()->Height(newHeight);
		m_Serialize = true;
	}


	auto Settings::RenderMultiplier() const noexcept -> float
	{
		return m_WindowSettingsCache.RenderMultiplier;
	}


	auto Settings::RenderMultiplier(float const newMult) noexcept -> void
	{
		m_WindowSettingsCache.RenderMultiplier = newMult;
		Window::Instance()->RenderMultiplier(newMult);
		m_Serialize = true;
	}


	auto Settings::Fullscreen() const noexcept -> bool
	{
		return m_WindowSettingsCache.Fullscreen;
	}


	auto Settings::Fullscreen(bool const newVal) noexcept -> void
	{
		m_WindowSettingsCache.Fullscreen = newVal;
		Window::Instance()->Fullscreen(newVal);
		m_Serialize = true;
	}


	auto Settings::Vsync() const -> bool
	{
		return m_WindowSettingsCache.Vsync;
	}


	LEOPPHAPI auto Settings::Vsync(bool const newVal) -> void
	{
		m_WindowSettingsCache.Vsync = newVal;
		Window::Instance()->Vsync(newVal);
		m_Serialize = true;
	}


	auto Settings::Serialize() const -> void
	{
		nlohmann::json json;

		// Serialize common global settings
		json[JSON_SHADER_LOC] = m_ShaderCache.string();
		json[JSON_API] = m_RenderingSettings.PendingApi;
		json[JSON_PIPE] = m_RenderingSettings.PendingPipe;
		for (auto i = 0u; i < m_DirLightSettings.Res.size(); i++)
		{
			json[JSON_DIR_SHADOW_RES][i] = m_DirLightSettings.Res[i];
		}
		json[JSON_DIR_CORRECT] = m_DirLightSettings.Corr;
		json[JSON_SPOT_SHADOW_RES] = m_SpotLightSettings.Res;
		json[JSON_MAX_SPOT] = m_SpotLightSettings.MaxNum;
		json[JSON_POINT_SHADOW_RES] = m_PointLightSettings.Res;
		json[JSON_MAX_POINT] = m_PointLightSettings.MaxNum;

		// Serialize window settings
		json[JSON_RES_W] = m_WindowSettingsCache.Width;
		json[JSON_RES_H] = m_WindowSettingsCache.Height;
		json[JSON_REND_MULT] = m_WindowSettingsCache.RenderMultiplier;
		json[JSON_FULLSCREEN] = m_WindowSettingsCache.Fullscreen;
		json[JSON_VSYNC] = m_WindowSettingsCache.Vsync;

		std::ofstream output{s_FilePath};
		output << json.dump(1);
		internal::Logger::Instance().Debug("Settings serialized to " + s_FilePath.string() + ".");
	}


	auto Settings::Deserialize() -> void
	{
		try
		{
			auto const json{
				[&]
				{
					std::ifstream input{s_FilePath};
					nlohmann::json ret;
					input >> ret;
					return ret;
				}()
			};

			// Parse common global settings
			m_ShaderCache = std::filesystem::path{json[JSON_SHADER_LOC].get<std::string>()};
			m_RenderingSettings.Api = json[JSON_API];
			m_RenderingSettings.PendingApi = m_RenderingSettings.Api;
			m_RenderingSettings.Pipe = json[JSON_PIPE];
			m_RenderingSettings.PendingPipe = m_RenderingSettings.Pipe;
			m_DirLightSettings.Res.clear();
			for (auto const& res : json[JSON_DIR_SHADOW_RES])
			{
				m_DirLightSettings.Res.push_back(res);
			}
			m_DirLightSettings.Corr = json[JSON_DIR_CORRECT];
			m_SpotLightSettings.Res = json[JSON_SPOT_SHADOW_RES];
			m_SpotLightSettings.MaxNum = json[JSON_MAX_SPOT];
			m_PointLightSettings.Res = json[JSON_POINT_SHADOW_RES];
			m_PointLightSettings.MaxNum = json[JSON_MAX_POINT];

			// Parse window settings
			m_WindowSettingsCache = WindowSettings
			{
				.Width = json[JSON_RES_W],
				.Height = json[JSON_RES_H],
				.RenderMultiplier = json[JSON_REND_MULT],
				.Fullscreen = json[JSON_FULLSCREEN],
				.Vsync = json[JSON_VSYNC]
			};

			internal::Logger::Instance().Debug("Settings deserialized from " + s_FilePath.string() + ".");
		}
		catch (std::exception&)
		{
			internal::Logger::Instance().Debug("Failed to deserialize from " + s_FilePath.string() + ". Reverting to defaults and overwriting config file.");
			m_Serialize = true;
		}
	}


	auto Settings::OnEventReceived(EventReceiver<internal::FrameCompleteEvent>::EventParamType) -> void
	{
		if (m_Serialize)
		{
			Serialize();
			m_Serialize = false;
		}
	}


	auto Settings::OnEventReceived(EventReceiver<internal::WindowEvent>::EventParamType event) -> void
	{
		m_WindowSettingsCache = WindowSettings
		{
			.Width = event.Width,
			.Height = event.Height,
			.RenderMultiplier = event.RenderMultiplier,
			.Fullscreen = event.Fullscreen,
			.Vsync = event.Vsync
		};

		m_Serialize = true;
	}
}
