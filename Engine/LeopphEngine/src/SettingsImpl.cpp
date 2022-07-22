#include "SettingsImpl.hpp"

#include "Logger.hpp"

#include <fstream>
#include <json.hpp>


namespace leopph::internal
{
	SettingsImpl::SettingsImpl()
	{
		if (exists(s_FilePath))
		{
			try
			{
				Deserialize();
				Logger::Instance().Debug("Settings successfully parsed from " + s_FilePath.generic_string() + ".");
			}
			catch (...)
			{
				Logger::Instance().Debug("Failed to parse settings from " + s_FilePath.generic_string() + ". Reverting to defaults.");
				m_Serialize = true;
			}
		}
		else
		{
			// Deferred serialization to that to-be-serialized objects surely exist and are initialized.
			m_Serialize = true;
		}
	}



	void SettingsImpl::Serialize() const
	{
		nlohmann::json json;

		json[JSON_SHADER_LOC] = m_ShaderCache.string();
		json[JSON_API] = m_RenderingSettings.PendingApi;
		json[JSON_PIPE] = m_RenderingSettings.PendingPipe;
		json[JSON_GAMMA] = m_RenderingSettings.Gamma;

		json[JSON_DIR_CORRECT] = m_DirLightSettings.Corr;
		for (auto i = 0u; i < m_DirLightSettings.Res.size(); i++)
		{
			json[JSON_DIR_SHADOW_RES][i] = m_DirLightSettings.Res[i];
		}

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
	}



	void SettingsImpl::Deserialize()
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

		m_ShaderCache = std::filesystem::path{json.at(JSON_SHADER_LOC).get<std::string>()};

		m_RenderingSettings.Api = json.at(JSON_API);
		m_RenderingSettings.Pipe = json.at(JSON_PIPE);
		m_RenderingSettings.Gamma = json.at(JSON_GAMMA);
		m_RenderingSettings.PendingApi = m_RenderingSettings.Api;
		m_RenderingSettings.PendingPipe = m_RenderingSettings.Pipe;

		m_DirLightSettings.Res.clear();
		m_DirLightSettings.Corr = json.at(JSON_DIR_CORRECT);
		for (auto const& res : json.at(JSON_DIR_SHADOW_RES))
		{
			m_DirLightSettings.Res.push_back(res);
		}

		m_SpotLightSettings.Res = json.at(JSON_SPOT_SHADOW_RES);
		m_SpotLightSettings.MaxNum = json.at(JSON_MAX_SPOT);

		m_PointLightSettings.Res = json.at(JSON_POINT_SHADOW_RES);
		m_PointLightSettings.MaxNum = json.at(JSON_MAX_POINT);

		m_WindowSettingsCache = WindowSettings
		{
			.Width = json.at(JSON_RES_W),
			.Height = json.at(JSON_RES_H),
			.RenderMultiplier = json.at(JSON_REND_MULT),
			.Fullscreen = json.at(JSON_FULLSCREEN),
			.Vsync = json.at(JSON_VSYNC)
		};
	}



	void SettingsImpl::OnEventReceived(EventReceiver<FrameCompleteEvent>::EventParamType)
	{
		if (m_Serialize)
		{
			Serialize();
			Logger::Instance().Debug("Settings serialized to " + s_FilePath.string() + ".");
			m_Serialize = false;
		}
	}



	void SettingsImpl::OnEventReceived(EventReceiver<WindowEvent>::EventParamType event)
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



	char const* const SettingsImpl::JSON_SHADER_LOC{"shaderCacheLoc"};
	char const* const SettingsImpl::JSON_DIR_SHADOW_RES{"dirShadowRes"};
	char const* const SettingsImpl::JSON_SPOT_SHADOW_RES{"spotShadowRes"};
	char const* const SettingsImpl::JSON_POINT_SHADOW_RES{"pointShadowRes"};
	char const* const SettingsImpl::JSON_MAX_SPOT{"numMaxSpot"};
	char const* const SettingsImpl::JSON_MAX_POINT{"numMaxPoint"};
	char const* const SettingsImpl::JSON_API{"api"};
	char const* const SettingsImpl::JSON_PIPE{"pipeline"};
	char const* const SettingsImpl::JSON_DIR_CORRECT{"dirShadowCascadeCorrection"};
	char const* const SettingsImpl::JSON_VSYNC{"vsync"};
	char const* const SettingsImpl::JSON_RES_W{"resW"};
	char const* const SettingsImpl::JSON_RES_H{"resH"};
	char const* const SettingsImpl::JSON_REND_MULT{"resMult"};
	char const* const SettingsImpl::JSON_FULLSCREEN{"fullscreen"};
	char const* const SettingsImpl::JSON_GAMMA{"gamma"};
}
