#pragma once

#include "../api/leopphapi.h"

#include <filesystem>

namespace leopph
{
	/*-------------------------------------------------------------------------------------
	The Settings class stores all LeopphEngine-related configurations.
	You can safely change these at runtime, though some may require an application restart.
	-------------------------------------------------------------------------------------*/
	// TODO parse settings from file

	class Settings
	{
	public:
		enum class GraphicsAPI
		{
			OpenGL
		};

		/* You can set whether shaders should compile on every run
		or they're cached on disk. If you choose to cache,
		you have to provide LeopphEngine with a directory.
		Requires restart to take effect. */
		LEOPPHAPI static bool IsCachingShaders();
		LEOPPHAPI static const std::filesystem::path& ShaderCacheLocation();
		LEOPPHAPI static void ShaderCacheLocation(std::filesystem::path path);

		/* You can check and change the currently used graphics API.
		Changing this setting requires an application restart. */
		static LEOPPHAPI const GraphicsAPI RenderAPI;
		static LEOPPHAPI void SetRenderAPI(GraphicsAPI newAPI);

		/* You can turn on or off vertical synchronization for the application window.
		You can also check its status. */
		static LEOPPHAPI bool Vsync();
		static LEOPPHAPI void Vsync(bool value);

	private:
		static std::filesystem::path s_ShaderLocation;
		static GraphicsAPI s_PendingRenderAPI;
	};
}