#pragma once

#include "../api/LeopphApi.hpp"
#include "../events/FrameEndEvent.hpp"
#include "../events/WindowEvent.hpp"
#include "../events/handling/EventReceiver.hpp"

#include <cstddef>
#include <filesystem>
#include <span>
#include <utility>
#include <vector>


namespace leopph
{
	// The Settings class provides access LeopphEngine-related configurations.
	// You can safely change these at runtime, though some may require an application restart.
	class Settings final : public EventReceiver<internal::FrameEndedEvent>, public EventReceiver<internal::WindowEvent>
	{
		public:
			// Enum for the different supported graphics APIs.
			enum class GraphicsApi
			{
				OpenGl
			};


			// Enum for the different supported graphics techniques.
			enum class GraphicsPipeline
			{
				Forward, Deferred
			};


			[[nodiscard]] LEOPPHAPI static
			auto Instance() -> Settings&;

			// Get where on the disk shaders are cached after compilation.
			// An empty location is returned if shader caching is turned off.
			[[nodiscard]] constexpr
			auto ShaderCachePath() noexcept -> const auto&;

			// Set where on the disk shaders are cached after compilation.
			// Setting this property to an empty path turns shader caching off and setting a valid value turns it on.
			inline
			auto ShaderCachePath(std::filesystem::path path) noexcept;

			// Get whether shaders are cached after compilation, or recompiled during each run.
			[[nodiscard]] LEOPPHAPI
			auto CacheShaders() const -> bool;

			// Get the currently used graphics API.
			[[nodiscard]] constexpr
			auto GetGraphicsApi() const noexcept;

			// Set the currently used graphics API.
			// Your application must be restarted before the new value takes effect.
			constexpr
			auto SetGraphicsApi(GraphicsApi newApi) noexcept;

			// Get the currently used graphics pipeline.
			[[nodiscard]] constexpr
			auto GetGraphicsPipeline() const noexcept;

			// Set the currently used graphics pipeline.
			// Your application must be restarted before the new value takes effect.
			constexpr
			auto SetGraphicsPipeline(GraphicsPipeline pipeline) noexcept;

			// Get the resolution of the shadow maps used by DirectionalLights.
			// Resolutions are returned in the order of the cascades that use them.
			// More values mean more cascade splits.
			// Higher values produce better quality shadows but increase VRAM and computation costs.
			[[nodiscard]] constexpr
			auto DirShadowResolution() -> const auto&;

			// Set the resolution of the shadow maps used by DirectionalLights.
			// Resolutions are accepted in the order of the cascades that use them.
			// More values mean more cascade splits.
			// Higher values produce better quality shadows but increase VRAM and computation costs.
			LEOPPHAPI
			auto DirShadowResolution(std::span<const std::size_t> cascades) -> void;

			// Get the correction factor when calculating shadow cascade bounds for DirectionalLights.
			[[nodiscard]] constexpr
			auto DirShadowCascadeCorrection() const noexcept;

			// Set the correction factor when calculating shadow cascade bounds for DirectionalLights.
			constexpr
			auto DirShadowCascadeCorrection(float newCor) noexcept;

			// Get the current number of shadow cascades DirectionalLights use.
			// This is the same as the size of the container set and returned by Settings::DirShadowResolution.
			[[nodiscard]] constexpr
			auto DirShadowCascadeCount() const noexcept;

			// Get the resolution of the shadow maps used by SpotLights.
			// Higher values produce sharper shadows but require more VRAM.
			[[nodiscard]] constexpr
			auto SpotShadowResolution() const noexcept;

			// Set the resolution of the shadow maps used by SpotLights.
			// Higher values produce sharper shadows but require more VRAM.
			LEOPPHAPI
			auto SpotShadowResolution(std::size_t newRes) -> void;

			// Get the maximum number of SpotLights that will be used in lighting calculations.
			// If there are more SpotLights in the scene than this number, LeopphEngine uses the ones closest to the active Camera.
			// Higher values mean more detailed lighting but can significantly reduce performance.
			[[nodiscard]] constexpr
			auto MaxSpotLightCount() const noexcept;

			// Set the maximum number of SpotLights that will be used in lighting calculations.
			// If there are more SpotLights in the scene than this number, LeopphEngine uses the ones closest to the active Camera.
			// Higher values mean more detailed lighting but can significantly reduce performance.
			constexpr
			auto MaxSpotLightCount(std::size_t newCount) noexcept;

			// Get the resolution of the shadow maps used by PointLights.
			// Higher values produce sharper shadows but require more VRAM.
			[[nodiscard]] constexpr
			auto PointShadowResolution() const noexcept;

			// Set the resolution of the shadow map used by PointLights.
			// Higher values produce sharper shadows but require more VRAM.
			LEOPPHAPI
			auto PointShadowResolution(std::size_t newRes) noexcept -> void;

			// Get the maximum number of PointLights that will be used in lighting calculations.
			// If there are more PointLights in the scene than this number, LeopphEngine uses the ones closest to the active Camera.
			// Higher values mean more detailed lighting but can significantly reduce performance.
			[[nodiscard]] constexpr
			auto MaxPointLightCount() const noexcept;

			// Set the maximum number of PointLights that will be used in lighting calculations.
			// If there are more PointLights in the scene than this number, LeopphEngine uses the ones closest to the active Camera.
			// Higher values mean more detailed lighting but can significantly reduce performance.
			constexpr
			auto MaxPointLightCount(std::size_t newCount) noexcept;

			// Get the width of the current window. This is the same as Window::Width.
			[[nodiscard]] LEOPPHAPI
			auto WindowWidth() const noexcept -> unsigned;

			// Set the width of the current window. This is the same as Window::Width.
			LEOPPHAPI
			auto WindowWidth(unsigned newWidth) noexcept -> void;

			// Get the height of the current window. This is the same as Window::Height.
			[[nodiscard]] LEOPPHAPI
			auto WindowHeight() const noexcept -> unsigned;

			// Set the height of the current window. This is the same as Window::Height.
			LEOPPHAPI
			auto WindowHeight(unsigned newHeight) noexcept -> void;

			// Get whether the current window has exclusive access to the monitor. This is the same as Window::Fullscreen.
			[[nodiscard]] LEOPPHAPI
			auto Fullscreen() const noexcept -> bool;

			// Set whether the window should have exclusive access to the monitor. This is the same as Window::Fullscreen.
			LEOPPHAPI
			auto Fullscreen(bool newVal) noexcept -> void;

			// Get the current render multiplier. This is the same as Window::RenderMultiplier.
			[[nodiscard]] LEOPPHAPI
			auto RenderMultiplier() const noexcept -> float;

			// Set the render multiplier. This is the same as Window::RenderMultiplier.
			LEOPPHAPI
			auto RenderMultiplier(float newMult) noexcept -> void;

			// Get whether Vsync is turned on.
			// This is exactly the same value as what Window::Vsync returns.
			[[nodiscard]] LEOPPHAPI
			auto Vsync() const -> bool;

			// Set whether Vsync is turned on.
			// This has exactly the same effect as using Window::Vsync.
			LEOPPHAPI
			auto Vsync(bool newVal) -> void;

			Settings(const Settings& other) = delete;
			auto operator=(const Settings& other) -> Settings& = delete;

			Settings(Settings&& other) noexcept = delete;
			auto operator=(Settings&& other) noexcept -> Settings& = delete;

			~Settings() noexcept override = default;

		private:
			Settings();
			auto Serialize() const -> void;
			auto Deserialize() -> void;
			// If serialization needs to happen, we do it at the end of frame.
			auto OnEventReceived(EventReceiver<internal::FrameEndedEvent>::EventParamType) -> void override;
			// Updates local window settings cache when serializable window configuration changes happen.
			auto OnEventReceived(EventReceiver<internal::WindowEvent>::EventParamType event) -> void override;

			// Cache to store serializable window configs.
			// Needs to be kept in sync with Window::Instance!
			struct WindowSettings
			{
				unsigned Width{960};
				unsigned Height{540};
				float RenderMultiplier{1};
				bool Fullscreen{false};
				bool Vsync{false};
			} m_WindowSettingsCache;


			struct RenderingSettings
			{
				GraphicsApi Api{GraphicsApi::OpenGl};
				GraphicsApi PendingApi{Api};
				GraphicsPipeline Pipe{GraphicsPipeline::Deferred};
				GraphicsPipeline PendingPipe{Pipe};
			} m_RenderingSettings;


			struct DirLightSettings
			{
				std::vector<std::size_t> Res{4096, 2048, 1024};
				float Corr{.75f};
			} m_DirLightSettings;


			struct SpotLightSettings
			{
				std::size_t Res{1024};
				std::size_t MaxNum{16};
			} m_SpotLightSettings;


			struct PointLightSettings
			{
				std::size_t Res{2048};
				std::size_t MaxNum{16};
			} m_PointLightSettings;


			std::filesystem::path m_ShaderCache;
			// When a value changes this is set to true
			// serialization will happen at the end of the frame
			bool m_Serialize{false};

			static std::filesystem::path s_FilePath;

			constexpr static const char* JSON_SHADER_LOC = "shaderCacheLoc";
			constexpr static const char* JSON_DIR_SHADOW_RES = "dirShadowRes";
			constexpr static const char* JSON_SPOT_SHADOW_RES = "spotShadowRes";
			constexpr static const char* JSON_POINT_SHADOW_RES = "pointShadowRes";
			constexpr static const char* JSON_MAX_SPOT = "numMaxSpot";
			constexpr static const char* JSON_MAX_POINT = "numMaxPoint";
			constexpr static const char* JSON_API = "api";
			constexpr static const char* JSON_PIPE = "pipeline";
			constexpr static const char* JSON_DIR_CORRECT = "dirShadowCascadeCorrection";
			constexpr static const char* JSON_VSYNC = "vsync";
			constexpr static const char* JSON_RES_W = "resW";
			constexpr static const char* JSON_RES_H = "resH";
			constexpr static const char* JSON_REND_MULT = "resMult";
			constexpr static const char* JSON_FULLSCREEN = "fullscreen";
	};


	constexpr auto Settings::ShaderCachePath() noexcept -> const auto&
	{
		return m_ShaderCache;
	}


	inline auto Settings::ShaderCachePath(std::filesystem::path path) noexcept
	{
		m_ShaderCache = std::move(path);
		m_Serialize = true;
	}


	constexpr auto Settings::GetGraphicsApi() const noexcept
	{
		return m_RenderingSettings.Api;
	}


	constexpr auto Settings::SetGraphicsApi(const GraphicsApi newApi) noexcept
	{
		m_RenderingSettings.PendingApi = newApi;
		m_Serialize = true;
	}


	constexpr auto Settings::GetGraphicsPipeline() const noexcept
	{
		return m_RenderingSettings.Pipe;
	}


	constexpr auto Settings::SetGraphicsPipeline(const GraphicsPipeline pipeline) noexcept
	{
		m_RenderingSettings.PendingPipe = pipeline;
		m_Serialize = true;
	}


	constexpr auto Settings::DirShadowResolution() -> const auto&
	{
		return m_DirLightSettings.Res;
	}


	constexpr auto Settings::DirShadowCascadeCorrection() const noexcept
	{
		return m_DirLightSettings.Corr;
	}


	constexpr auto Settings::DirShadowCascadeCorrection(const float newCor) noexcept
	{
		m_DirLightSettings.Corr = newCor;
		m_Serialize = true;
	}


	constexpr auto Settings::DirShadowCascadeCount() const noexcept
	{
		return m_DirLightSettings.Res.size();
	}


	constexpr auto Settings::SpotShadowResolution() const noexcept
	{
		return m_SpotLightSettings.Res;
	}


	constexpr auto Settings::MaxSpotLightCount() const noexcept
	{
		return m_SpotLightSettings.MaxNum;
	}


	constexpr auto Settings::MaxSpotLightCount(const std::size_t newCount) noexcept
	{
		m_SpotLightSettings.MaxNum = newCount;
		m_Serialize = true;
	}


	constexpr auto Settings::PointShadowResolution() const noexcept
	{
		return m_PointLightSettings.Res;
	}


	constexpr auto Settings::MaxPointLightCount() const noexcept
	{
		return m_PointLightSettings.MaxNum;
	}


	constexpr auto Settings::MaxPointLightCount(const std::size_t newCount) noexcept
	{
		m_PointLightSettings.MaxNum = newCount;
		m_Serialize = true;
	}
}
