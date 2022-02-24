#include "Settings.hpp"

#include "../events/DirShadowResChangeEvent.hpp"
#include "../events/SpotShadowResolutionEvent.hpp"
#include "../events/handling/EventManager.hpp"
#include "../util/logger.h"
#include "../windowing/WindowBase.hpp"

#include <fstream>
#include <json.hpp>


namespace leopph
{
	std::filesystem::path Settings::s_FilePath{"settings.json"};


	Settings::Settings()
	{
		if (std::filesystem::exists(s_FilePath))
		{
			Deserialize();
		}
		else
		{
			Serialize();
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


	LEOPPHAPI auto Settings::Vsync() const -> bool
	{
		return m_Vsync;
	}


	auto Settings::WindowWidth(const float newWidth) noexcept -> void
	{
		m_WindowRes[0] = newWidth;
		EventManager::Instance().Send<internal::WindowEvent>(m_WindowRes, m_RenderMult, m_Vsync, m_Fullscreen);
		m_Serialize = true;
	}


	auto Settings::WindowHeight(const float newHeight) noexcept -> void
	{
		m_WindowRes[1] = newHeight;
		EventManager::Instance().Send<internal::WindowEvent>(m_WindowRes, m_RenderMult, m_Vsync, m_Fullscreen);
		m_Serialize = true;
	}


	auto Settings::Fullscreen(const bool newVal) noexcept -> void
	{
		m_Fullscreen = newVal;
		EventManager::Instance().Send<internal::WindowEvent>(m_WindowRes, m_RenderMult, m_Vsync, m_Fullscreen);
		m_Serialize = true;
	}


	auto Settings::RenderMultiplier(const float newMult) noexcept -> void
	{
		m_RenderMult = newMult;
		EventManager::Instance().Send<internal::WindowEvent>(m_WindowRes, m_RenderMult, m_Vsync, m_Fullscreen);
		m_Serialize = true;
	}


	LEOPPHAPI auto Settings::Vsync(const bool value) -> void
	{
		m_Vsync = value;
		EventManager::Instance().Send<internal::WindowEvent>(m_WindowRes, m_RenderMult, m_Vsync, m_Fullscreen);
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
		json[JSON_VSYNC] = m_Vsync;
		json[JSON_RES_W] = m_WindowRes[0];
		json[JSON_RES_H] = m_WindowRes[1];
		json[JSON_RES_MULT] = m_RenderMult;
		json[JSON_FULLSCREEN] = m_Fullscreen;
		std::ofstream output{s_FilePath};
		output << json.dump(1);
		internal::Logger::Instance().Debug("Settings serialized to " + s_FilePath.string() + ".");
	}


	auto Settings::Deserialize() -> void
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
		m_Vsync = json[JSON_VSYNC];
		m_WindowRes[0] = json[JSON_RES_W];
		m_WindowRes[1] = json[JSON_RES_H];
		m_RenderMult = json[JSON_RES_MULT];
		m_Fullscreen = json[JSON_FULLSCREEN];

		internal::Logger::Instance().Debug("Settings deserialized from " + s_FilePath.string() + ".");
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
		m_WindowRes = event.Resolution;
		m_Vsync = event.Vsync;
		m_Fullscreen = event.Fullscreen;
		m_RenderMult = event.RenderMultiplier;

		m_Serialize = true;
	}
}
