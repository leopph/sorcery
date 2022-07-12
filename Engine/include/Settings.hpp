#pragma once

#include "LeopphApi.hpp"
#include "Types.hpp"

#include <cstddef>
#include <filesystem>
#include <span>
#include <utility>
#include <vector>


namespace leopph
{
	// The Settings class provides access LeopphEngine-related configurations.
	// You can safely change these at runtime, though some may require an application restart.
	class Settings
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


			// Get where on the disk shaders are cached after compilation.
			// An empty location is returned if shader caching is turned off.
			[[nodiscard]] auto LEOPPHAPI ShaderCachePath() noexcept -> std::filesystem::path const&;


			// Set where on the disk shaders are cached after compilation.
			// Setting this property to an empty path turns shader caching off and setting a valid value turns it on.
			auto LEOPPHAPI ShaderCachePath(std::filesystem::path path) noexcept -> void;


			// Get whether shaders are cached after compilation, or recompiled during each run.
			[[nodiscard]] auto LEOPPHAPI CacheShaders() const -> bool;


			// Get the currently used graphics API.
			[[nodiscard]] auto LEOPPHAPI GetGraphicsApi() const noexcept -> GraphicsApi;


			// Set the currently used graphics API.
			// Your application must be restarted before the new value takes effect.
			auto LEOPPHAPI SetGraphicsApi(GraphicsApi newApi) noexcept -> void;


			// Get the currently used graphics pipeline.
			[[nodiscard]] auto LEOPPHAPI GetGraphicsPipeline() const noexcept -> GraphicsPipeline;


			// Set the currently used graphics pipeline.
			// Your application must be restarted before the new value takes effect.
			auto LEOPPHAPI SetGraphicsPipeline(GraphicsPipeline pipeline) noexcept -> void;


			// Get the resolution of the shadow cascade maps used by DirectionalLights.
			// They are returned in order from the closest cascade to the farthest.
			// More values mean more cascades.
			// Higher values produce better quality shadows but increase VRAM and computation costs.
			[[nodiscard]] auto LEOPPHAPI DirShadowResolution() -> std::span<u16 const>;


			// Set the resolution of the shadow cascade maps used by DirectionalLights.
			// They are accepted in order from the closest cascade to the farthest.
			// More values mean more cascades.
			// Higher values produce better quality shadows but increase VRAM and computation costs.
			// The number of resolutions accepted is capped.
			auto LEOPPHAPI DirShadowResolution(std::span<u16 const> cascades) -> void;


			// Get the correction factor when calculating shadow cascade bounds for DirectionalLights.
			[[nodiscard]] auto LEOPPHAPI DirShadowCascadeCorrection() const noexcept -> f32;


			// Set the correction factor when calculating shadow cascade bounds for DirectionalLights.
			auto LEOPPHAPI DirShadowCascadeCorrection(f32 newCor) noexcept -> void;


			// Get the number of shadow cascade maps used by DirectionalLights.
			// This is the same as the size of the container returned by Settings::DirShadowResolution.
			[[nodiscard]] auto LEOPPHAPI DirShadowCascadeCount() const noexcept -> u8;


			// Get the resolution of the shadow maps used by SpotLights.
			// Higher values produce sharper shadows but require more VRAM.
			[[nodiscard]] auto LEOPPHAPI SpotShadowResolution() const noexcept -> u16;


			// Set the resolution of the shadow maps used by SpotLights.
			// Higher values produce sharper shadows but require more VRAM.
			auto LEOPPHAPI SpotShadowResolution(u16 newRes) -> void;


			// Get the maximum number of SpotLights that will be used in lighting calculations.
			// If there are more SpotLights in the scene than this number, LeopphEngine uses the ones closest to the active Camera.
			// Higher values mean more detailed lighting but can significantly reduce performance.
			[[nodiscard]] auto LEOPPHAPI MaxSpotLightCount() const noexcept -> u8;


			// Set the maximum number of SpotLights that will be used in lighting calculations.
			// If there are more SpotLights in the scene than this number, LeopphEngine uses the ones closest to the active Camera.
			// Higher values mean more detailed lighting but can significantly reduce performance.
			auto LEOPPHAPI MaxSpotLightCount(u8 newCount) noexcept -> void;


			// Get the resolution of the shadow maps used by PointLights.
			// Higher values produce sharper shadows but require more VRAM.
			[[nodiscard]] auto LEOPPHAPI PointShadowResolution() const noexcept -> u16;


			// Set the resolution of the shadow map used by PointLights.
			// Higher values produce sharper shadows but require more VRAM.
			auto LEOPPHAPI PointShadowResolution(u16 newRes) noexcept -> void;


			// Get the maximum number of PointLights that will be used in lighting calculations.
			// If there are more PointLights in the scene than this number, LeopphEngine uses the ones closest to the active Camera.
			// Higher values mean more detailed lighting but can significantly reduce performance.
			[[nodiscard]] auto LEOPPHAPI MaxPointLightCount() const noexcept -> u8;


			// Set the maximum number of PointLights that will be used in lighting calculations.
			// If there are more PointLights in the scene than this number, LeopphEngine uses the ones closest to the active Camera.
			// Higher values mean more detailed lighting but can significantly reduce performance.
			auto LEOPPHAPI MaxPointLightCount(u8 newCount) noexcept;


			// Get the width of the current window. This is the same as Window::Width.
			[[nodiscard]] auto LEOPPHAPI WindowWidth() const noexcept -> u32;


			// Set the width of the current window. This is the same as Window::Width.
			auto LEOPPHAPI WindowWidth(u32 newWidth) noexcept -> void;


			// Get the height of the current window. This is the same as Window::Height.
			[[nodiscard]] auto LEOPPHAPI WindowHeight() const noexcept -> u32;


			// Set the height of the current window. This is the same as Window::Height.
			auto LEOPPHAPI WindowHeight(u32 newHeight) noexcept -> void;


			// Get the current render multiplier. This is the same as Window::RenderMultiplier.
			[[nodiscard]] auto LEOPPHAPI RenderMultiplier() const noexcept -> f32;


			// Set the render multiplier. This is the same as Window::RenderMultiplier.
			auto LEOPPHAPI RenderMultiplier(f32 newMult) noexcept -> void;


			// Get whether the current window has exclusive access to the monitor. This is the same as Window::Fullscreen.
			[[nodiscard]] auto LEOPPHAPI Fullscreen() const noexcept -> bool;


			// Set whether the window should have exclusive access to the monitor. This is the same as Window::Fullscreen.
			auto LEOPPHAPI Fullscreen(bool newVal) noexcept -> void;


			// Get whether Vsync is turned on.
			// This is exactly the same value as what Window::Vsync returns.
			[[nodiscard]] auto LEOPPHAPI Vsync() const -> bool;


			// Set whether Vsync is turned on.
			// This has exactly the same effect as using Window::Vsync.
			auto LEOPPHAPI Vsync(bool newVal) -> void;


			// Get the gamma correction value.
			auto LEOPPHAPI Gamma() const -> f32;


			// Set the gamma correction value.
			auto LEOPPHAPI Gamma(f32 newVal) -> void;


			Settings(Settings const& other) = delete;
			auto operator=(Settings const& other) -> Settings& = delete;

			Settings(Settings&& other) noexcept = delete;
			auto operator=(Settings&& other) noexcept -> Settings& = delete;


		protected:
			Settings() noexcept = default;
			~Settings() noexcept = default;


			// Cache to store serializable window configs.
			// Needs to be kept in sync with Window::Instance!
			struct WindowSettings
			{
				u32 Width{960};
				u32 Height{540};
				f32 RenderMultiplier{1};
				bool Fullscreen{false};
				bool Vsync{false};
			} m_WindowSettingsCache;



			struct RenderingSettings
			{
				GraphicsApi Api{GraphicsApi::OpenGl};
				GraphicsApi PendingApi{Api};
				GraphicsPipeline Pipe{GraphicsPipeline::Deferred};
				GraphicsPipeline PendingPipe{Pipe};
				f32 Gamma{2.2f};
			} m_RenderingSettings;



			struct DirLightSettings
			{
				std::vector<u16> Res{4096, 2048, 1024};
				f32 Corr{.75f};
			} m_DirLightSettings;



			struct SpotLightSettings
			{
				u16 Res{2048};
				u8 MaxNum{8};
			} m_SpotLightSettings;



			struct PointLightSettings
			{
				u16 Res{1024};
				u8 MaxNum{8};
			} m_PointLightSettings;


			std::filesystem::path m_ShaderCache;

			// When a value changes this is set to true
			// serialization will happen at the end of the frame
			bool m_Serialize{false};

			static std::filesystem::path s_FilePath;
	};
}
