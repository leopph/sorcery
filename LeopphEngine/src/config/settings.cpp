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
		/*const auto& window{internal::WindowBase::Get()};
		json[JSON_VSYNC] = window.Vsync();
		json[JSON_RES_W] = window.Width();
		json[JSON_RES_H] = window.Height();
		json[JSON_RES_MULT] = window.RenderMultiplier();
		json[JSON_FULLSCREEN] = window.Fullscreen();*/
		std::ofstream output{s_FilePath};
		output << json.dump(1);
		internal::Logger::Instance().Debug("Settings serialized to " + s_FilePath.string() + ".");
	}


	auto Settings::Deserialize() -> void
	{
		const auto json{[&]
		{
			std::ifstream input{s_FilePath};
			nlohmann::json ret;
			input >> ret;
			return ret;
		}()};
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
		/*auto& window{internal::WindowBase::Get()};
		window.Vsync(json[JSON_VSYNC]);
		window.Width(json[JSON_RES_W]);
		window.Height(json[JSON_RES_H]);
		window.RenderMultiplier(json[JSON_RES_MULT]);
		window.Fullscreen(json[JSON_FULLSCREEN]);*/

		internal::Logger::Instance().Debug("Settings deserialized from " + s_FilePath.string() + ".");
	}


	auto Settings::OnEventReceived(EventParamType) -> void
	{
		if (m_Serialize)
		{
			Serialize();
			m_Serialize = false;
		}
	}
}
