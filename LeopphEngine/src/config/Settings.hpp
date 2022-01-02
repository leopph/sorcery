#pragma once

#include "../api/LeopphApi.hpp"
#include "../misc/ShadowCascade.hpp"

#include <cstddef>
#include <filesystem>
#include <span>
#include <utility>
#include <vector>


namespace leopph
{
	// TODO parse settings from file
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


			// Enum for the different supported rendering techniques.
			enum class RenderType
			{
				Forward, Deferred
			};


			// Get where on the disk shaders are cached after compilation.
			// An empty location is returned if shader caching is turned off.
			[[nodiscard]] inline static auto ShaderCacheLocation() noexcept -> const auto&;

			// Get the currently used rendering technique.
			[[nodiscard]] inline static auto RenderingPipeline() noexcept;

			// Get the currently used rendering API.
			[[nodiscard]] inline static auto RenderingApi() noexcept;

			// Get the resolution of the shadow maps used by DirectionalLights.
			// Resolutions are returned in the order of the cascades that use them.
			// More values mean more cascade splits.
			// Higher values produce better quality shadows but increase VRAM and computation costs.
			[[nodiscard]] inline static auto DirShadowRes() -> const auto&;

			// Get the resolution of the shadows cast by SpotLights.
			// Higher values produce sharper shadows but require more VRAM.
			[[nodiscard]] inline static auto SpotLightShadowMapResolution() noexcept;

			// Get the resolution of the shadows cast by PointLights.
			// Higher values produce sharper shadows but require more VRAM.
			[[nodiscard]] inline static auto PointLightShadowMapResolution() noexcept;

			// Get the maximum number of SpotLights that will used in lighting calculations.
			// If there are more SpotLights in the scene than this number, LeopphEngine uses the ones closest to the active Camera.
			// Higher values mean more detailed lighting but can significantly reduce performance.
			[[nodiscard]] inline static auto MaxSpotLightCount() noexcept;

			// Get the maximum number of PointLights that will used in lighting calculations.
			// If there are more PointLights in the scene than this number, LeopphEngine uses the ones closest to the active Camera.
			// Higher values mean more detailed lighting but can significantly reduce performance.
			[[nodiscard]] inline static auto MaxPointLightCount() noexcept;

			// Get the correction factor when calculating shadow cascade bounds for DirectionalLights.
			inline static auto DirLightShadowCascadeCorrection() noexcept;

			// Get whether shaders are cached after compilation, or recompiled during each run.
			[[nodiscard]] LEOPPHAPI static auto CacheShaders() -> bool;

			// Get whether Vsync is turned on.
			// This is exactly the same value as what Window::Vsync returns.
			[[nodiscard]] LEOPPHAPI static auto Vsync() -> bool;

			// Get the current number of shadow cascades DirectionalLights use.
			// This is the same as the size of the container set and returned by Settings::DirShadowRes.
			[[nodiscard]] LEOPPHAPI static auto DirShadowCascadeCount() -> std::size_t;

			// Set where on the disk shaders are cached after compilation.
			// Setting this property to an empty path turns shader caching off and setting a valid value turns it on.
			inline static auto ShaderCacheLocation(std::filesystem::path path) noexcept;

			// Set the currently used rendering technique.
			// Your application must be restarted before the new value takes effect.
			inline static auto RenderingPipeline(RenderType type) noexcept;

			// Set the currently used graphics API.
			// Your application must be restarted before the new value takes effect.
			inline static auto RenderingApi(GraphicsApi newApi) noexcept;

			// Set the resolution of the shadows cast by PointLights.
			// Higher values produce sharper shadows but require more VRAM.
			inline static auto PointLightShadowMapResolution(std::size_t newRes) noexcept;

			// Set the maximum number of SpotLights that will used in lighting calculations.
			// If there are more SpotLights in the scene than this number, LeopphEngine uses the ones closest to the active Camera.
			// Higher values mean more detailed lighting but can significantly reduce performance.
			inline static auto MaxSpotLightCount(std::size_t newCount) noexcept;

			// Set the maximum number of PointLights that will used in lighting calculations.
			// If there are more PointLights in the scene than this number, LeopphEngine uses the ones closest to the active Camera.
			// Higher values mean more detailed lighting but can significantly reduce performance.
			inline static auto MaxPointLightCount(std::size_t newCount) noexcept;

			// Set the correction factor when calculating shadow cascade bounds for DirectionalLights.
			inline static auto DirLightShadowCascadeCorrection(float newCor) noexcept;

			// Set whether Vsync is turned on.
			// This has exactly the same effect as using Window::Vsync.
			LEOPPHAPI static auto Vsync(bool value) -> void;

			// Set the resolution of the shadow maps used by DirectionalLights.
			// Resolutions are accepted in the order of the cascades that use them.
			// More values mean more cascade splits.
			// Higher values produce better quality shadows but increase VRAM and computation costs.
			LEOPPHAPI static auto DirShadowRes(std::span<const std::size_t> cascades) -> void;

			// Set the resolution of the shadows cast by SpotLights.
			// Higher values produce sharper shadows but require more VRAM.
			LEOPPHAPI static auto SpotLightShadowMapResolution(std::size_t newRes) -> void;

		private:
			static std::filesystem::path s_CacheLoc;
			static std::vector<std::size_t> s_DirShadowRes;
			static std::size_t s_SpotShadowRes;
			static std::size_t s_PointShadowRes;
			static std::size_t s_NumMaxSpot;
			static std::size_t s_NumMaxPoint;
			static GraphicsApi s_Api;
			static GraphicsApi s_PendingApi;
			static RenderType s_Pipeline;
			static RenderType s_PendingPipeline;
			static float s_DirShadowCascadeCorrection;
	};


	inline auto Settings::ShaderCacheLocation() noexcept -> const auto&
	{
		return s_CacheLoc;
	}


	inline auto Settings::RenderingPipeline() noexcept
	{
		return s_Pipeline;
	}


	inline auto Settings::RenderingApi() noexcept
	{
		return s_Api;
	}


	inline auto Settings::DirShadowRes() -> const auto&
	{
		return s_DirShadowRes;
	}


	inline auto Settings::SpotLightShadowMapResolution() noexcept
	{
		return s_SpotShadowRes;
	}


	inline auto Settings::PointLightShadowMapResolution() noexcept
	{
		return s_PointShadowRes;
	}


	inline auto Settings::MaxSpotLightCount() noexcept
	{
		return s_NumMaxSpot;
	}


	inline auto Settings::MaxPointLightCount() noexcept
	{
		return s_NumMaxPoint;
	}


	inline auto Settings::DirLightShadowCascadeCorrection() noexcept
	{
		return s_DirShadowCascadeCorrection;
	}


	inline auto Settings::ShaderCacheLocation(std::filesystem::path path) noexcept
	{
		s_CacheLoc = std::move(path);
	}


	inline auto Settings::RenderingPipeline(const RenderType type) noexcept
	{
		s_PendingPipeline = type;
	}


	inline auto Settings::RenderingApi(const GraphicsApi newApi) noexcept
	{
		s_PendingApi = newApi;
	}


	inline auto Settings::PointLightShadowMapResolution(const std::size_t newRes) noexcept
	{
		s_PointShadowRes = newRes;
	}


	inline auto Settings::MaxSpotLightCount(const std::size_t newCount) noexcept
	{
		s_NumMaxSpot = newCount;
	}


	inline auto Settings::MaxPointLightCount(const std::size_t newCount) noexcept
	{
		s_NumMaxPoint = newCount;
	}


	inline auto Settings::DirLightShadowCascadeCorrection(const float newCor) noexcept
	{
		s_DirShadowCascadeCorrection = newCor;
	}
}
