#include "Settings.hpp"

#include "../events/DirShadowResChangeEvent.hpp"
#include "../events/SpotShadowResolutionEvent.hpp"
#include "../events/handling/EventManager.hpp"
#include "../util/Logger.hpp"
#include "../windowing/WindowImpl.hpp"

#include <exception>
#include <fstream>
#include <json.hpp>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Psapi.h>


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
		return !m_CacheLoc.empty();
	}


	auto Settings::WindowWidth() const noexcept -> unsigned
	{
		return m_WindowSettingsCache.Width;
	}


	auto Settings::WindowWidth(const unsigned newWidth) noexcept -> void
	{
		m_WindowSettingsCache.Width = newWidth;
		Window::Instance()->Width(newWidth);
		m_Serialize = true;
	}


	auto Settings::WindowHeight() const noexcept -> unsigned
	{
		return m_WindowSettingsCache.Height;
	}


	auto Settings::WindowHeight(const unsigned newHeight) noexcept -> void
	{
		m_WindowSettingsCache.Height = newHeight;
		Window::Instance()->Height(newHeight);
		m_Serialize = true;
	}


	auto Settings::Fullscreen() const noexcept -> bool
	{
		return m_WindowSettingsCache.Fullscreen;
	}


	auto Settings::Fullscreen(const bool newVal) noexcept -> void
	{
		m_WindowSettingsCache.Fullscreen = newVal;
		Window::Instance()->Fullscreen(newVal);
		m_Serialize = true;
	}


	auto Settings::RenderMultiplier() const noexcept -> float
	{
		return m_WindowSettingsCache.RenderMultiplier;
	}


	auto Settings::RenderMultiplier(const float newMult) noexcept -> void
	{
		m_WindowSettingsCache.RenderMultiplier = newMult;
		Window::Instance()->RenderMultiplier(newMult);
		m_Serialize = true;
	}


	auto Settings::Vsync() const -> bool
	{
		return m_WindowSettingsCache.Vsync;
	}


	LEOPPHAPI auto Settings::Vsync(const bool newVal) -> void
	{
		m_WindowSettingsCache.Vsync = newVal;
		Window::Instance()->Vsync(newVal);
		m_Serialize = true;
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


	auto Settings::Serialize() const -> void
	{
		nlohmann::json json;

		// Serialize common global settings
		json[JSON_SHADER_LOC] = m_CacheLoc.string();
		for (auto i = 0u; i < m_DirShadowRes.size(); i++)
		{
			json[JSON_DIR_SHADOW_RES][i] = m_DirShadowRes[i];
		}
		json[JSON_SPOT_SHADOW_RES] = m_SpotShadowRes;
		json[JSON_POINT_SHADOW_RES] = m_PointShadowRes;
		json[JSON_MAX_SPOT] = m_NumMaxSpot;
		json[JSON_MAX_POINT] = m_NumMaxPoint;
		json[JSON_API] = m_PendingApi;
		json[JSON_PIPE] = m_PendingPipeline;
		json[JSON_DIR_CORRECT] = m_DirShadowCascadeCorrection;

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
			const auto json{
				[&]
				{
					std::ifstream input{s_FilePath};
					nlohmann::json ret;
					input >> ret;
					return ret;
				}()
			};

			// Parse common global settings
			m_CacheLoc = std::filesystem::path{json[JSON_SHADER_LOC].get<std::string>()};
			m_DirShadowRes.clear();
			for (const auto& res : json[JSON_DIR_SHADOW_RES])
			{
				m_DirShadowRes.push_back(res);
			}
			m_SpotShadowRes = json[JSON_SPOT_SHADOW_RES];
			m_PointShadowRes = json[JSON_POINT_SHADOW_RES];
			m_NumMaxSpot = json[JSON_MAX_SPOT];
			m_NumMaxPoint = json[JSON_MAX_POINT];
			m_Api = json[JSON_API];
			m_PendingApi = m_Api;
			m_Pipeline = json[JSON_PIPE];
			m_PendingPipeline = m_Pipeline;
			m_DirShadowCascadeCorrection = json[JSON_DIR_CORRECT];

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


	auto Settings::OnEventReceived(EventReceiver<internal::FrameEndedEvent>::EventParamType) -> void
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
			.Width = event.Resolution[0],
			.Height = event.Resolution[1],
			.RenderMultiplier = event.RenderMultiplier,
			.Fullscreen = event.Fullscreen,
			.Vsync = event.Vsync
		};

		m_Serialize = true;
	}
}
