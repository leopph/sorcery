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
			[[nodiscard]] LEOPPHAPI std::filesystem::path const& ShaderCachePath() noexcept;


			// Set where on the disk shaders are cached after compilation.
			// Setting this property to an empty path turns shader caching off and setting a valid value turns it on.
			LEOPPHAPI void ShaderCachePath(std::filesystem::path path) noexcept;


			// Get whether shaders are cached after compilation, or recompiled during each run.
			[[nodiscard]] LEOPPHAPI bool CacheShaders() const;


			// Get the currently used graphics API.
			[[nodiscard]] LEOPPHAPI GraphicsApi GetGraphicsApi() const noexcept;


			// Set the currently used graphics API.
			// Your application must be restarted before the new value takes effect.
			LEOPPHAPI void SetGraphicsApi(GraphicsApi newApi) noexcept;


			// Get the currently used graphics pipeline.
			[[nodiscard]] LEOPPHAPI GraphicsPipeline GetGraphicsPipeline() const noexcept;


			// Set the currently used graphics pipeline.
			// Your application must be restarted before the new value takes effect.
			LEOPPHAPI void SetGraphicsPipeline(GraphicsPipeline pipeline) noexcept;


			// Get the resolution of the shadow cascade maps used by DirectionalLights.
			// They are returned in order from the closest cascade to the farthest.
			// More values mean more cascades.
			// Higher values produce better quality shadows but increase VRAM and computation costs.
			[[nodiscard]] LEOPPHAPI std::span<u16 const> DirShadowResolution();


			// Set the resolution of the shadow cascade maps used by DirectionalLights.
			// They are accepted in order from the closest cascade to the farthest.
			// More values mean more cascades.
			// Higher values produce better quality shadows but increase VRAM and computation costs.
			// The number of resolutions accepted is capped.
			LEOPPHAPI void DirShadowResolution(std::span<u16 const> cascades);


			// Get the correction factor when calculating shadow cascade bounds for DirectionalLights.
			[[nodiscard]] LEOPPHAPI f32 DirShadowCascadeCorrection() const noexcept;


			// Set the correction factor when calculating shadow cascade bounds for DirectionalLights.
			LEOPPHAPI void DirShadowCascadeCorrection(f32 newCor) noexcept;


			// Get the number of shadow cascade maps used by DirectionalLights.
			// This is the same as the size of the container returned by Settings::DirShadowResolution.
			[[nodiscard]] LEOPPHAPI u8 DirShadowCascadeCount() const noexcept;


			// Get the resolution of the shadow maps used by SpotLights.
			// Higher values produce sharper shadows but require more VRAM.
			[[nodiscard]] LEOPPHAPI u16 SpotShadowResolution() const noexcept;


			// Set the resolution of the shadow maps used by SpotLights.
			// Higher values produce sharper shadows but require more VRAM.
			LEOPPHAPI void SpotShadowResolution(u16 newRes);


			// Get the maximum number of SpotLights that will be used in lighting calculations.
			// If there are more SpotLights in the scene than this number, LeopphEngine uses the ones closest to the active Camera.
			// Higher values mean more detailed lighting but can significantly reduce performance.
			[[nodiscard]] LEOPPHAPI u8 MaxSpotLightCount() const noexcept;


			// Set the maximum number of SpotLights that will be used in lighting calculations.
			// If there are more SpotLights in the scene than this number, LeopphEngine uses the ones closest to the active Camera.
			// Higher values mean more detailed lighting but can significantly reduce performance.
			LEOPPHAPI void MaxSpotLightCount(u8 newCount) noexcept;


			// Get the resolution of the shadow maps used by PointLights.
			// Higher values produce sharper shadows but require more VRAM.
			[[nodiscard]] LEOPPHAPI u16 PointShadowResolution() const noexcept;


			// Set the resolution of the shadow map used by PointLights.
			// Higher values produce sharper shadows but require more VRAM.
			LEOPPHAPI void PointShadowResolution(u16 newRes) noexcept;


			// Get the maximum number of PointLights that will be used in lighting calculations.
			// If there are more PointLights in the scene than this number, LeopphEngine uses the ones closest to the active Camera.
			// Higher values mean more detailed lighting but can significantly reduce performance.
			[[nodiscard]] LEOPPHAPI u8 MaxPointLightCount() const noexcept;


			// Set the maximum number of PointLights that will be used in lighting calculations.
			// If there are more PointLights in the scene than this number, LeopphEngine uses the ones closest to the active Camera.
			// Higher values mean more detailed lighting but can significantly reduce performance.
			auto LEOPPHAPI MaxPointLightCount(u8 newCount) noexcept;


			// Get the width of the current window. This is the same as Window::Width.
			[[nodiscard]] LEOPPHAPI u32 WindowWidth() const noexcept;


			// Set the width of the current window. This is the same as Window::Width.
			LEOPPHAPI void WindowWidth(u32 newWidth) noexcept;


			// Get the height of the current window. This is the same as Window::Height.
			[[nodiscard]] LEOPPHAPI u32 WindowHeight() const noexcept;


			// Set the height of the current window. This is the same as Window::Height.
			LEOPPHAPI void WindowHeight(u32 newHeight) noexcept;


			// Get the current render multiplier. This is the same as Window::RenderMultiplier.
			[[nodiscard]] LEOPPHAPI f32 RenderMultiplier() const noexcept;


			// Set the render multiplier. This is the same as Window::RenderMultiplier.
			LEOPPHAPI void RenderMultiplier(f32 newMult) noexcept;


			// Get whether the current window has exclusive access to the monitor. This is the same as Window::Fullscreen.
			[[nodiscard]] LEOPPHAPI bool Fullscreen() const noexcept;


			// Set whether the window should have exclusive access to the monitor. This is the same as Window::Fullscreen.
			LEOPPHAPI void Fullscreen(bool newVal) noexcept;


			// Get whether Vsync is turned on.
			// This is exactly the same value as what Window::Vsync returns.
			[[nodiscard]] LEOPPHAPI bool Vsync() const;


			// Set whether Vsync is turned on.
			// This has exactly the same effect as using Window::Vsync.
			LEOPPHAPI void Vsync(bool newVal);


			// Get the gamma correction value.
			LEOPPHAPI f32 Gamma() const;


			// Set the gamma correction value.
			LEOPPHAPI void Gamma(f32 newVal);


			Settings(Settings const& other) = delete;
			Settings& operator=(Settings const& other) = delete;

			Settings(Settings&& other) noexcept = delete;
			Settings& operator=(Settings&& other) noexcept = delete;


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
