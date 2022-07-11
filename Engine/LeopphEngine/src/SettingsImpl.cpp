#include "SettingsImpl.hpp"

#include "Logger.hpp"

#include <exception>
#include <fstream>
#include <json.hpp>


namespace leopph::internal
{
	SettingsImpl::SettingsImpl()
	{
		if (exists(s_FilePath))
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



	auto SettingsImpl::Serialize() const -> void
	{
		nlohmann::json json;

		// Serialize common global settings
		json[JSON_SHADER_LOC] = m_ShaderCache.string();
		json[JSON_API] = m_RenderingSettings.PendingApi;
		json[JSON_PIPE] = m_RenderingSettings.PendingPipe;
		json[JSON_GAMMA] = m_RenderingSettings.Gamma;
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
		Logger::Instance().Debug("Settings serialized to " + s_FilePath.string() + ".");
	}



	auto SettingsImpl::Deserialize() -> void
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
			m_RenderingSettings.Gamma = json[JSON_GAMMA];
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

			Logger::Instance().Debug("Settings deserialized from " + s_FilePath.string() + ".");
		}
		catch (std::exception&)
		{
			Logger::Instance().Debug("Failed to deserialize from " + s_FilePath.string() + ". Reverting to defaults and overwriting config file.");
			m_Serialize = true;
		}
	}



	auto SettingsImpl::OnEventReceived(EventReceiver<FrameCompleteEvent>::EventParamType) -> void
	{
		if (m_Serialize)
		{
			Serialize();
			m_Serialize = false;
		}
	}



	auto SettingsImpl::OnEventReceived(EventReceiver<WindowEvent>::EventParamType event) -> void
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
