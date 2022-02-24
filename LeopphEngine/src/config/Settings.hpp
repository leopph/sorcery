#pragma once

#include "../api/LeopphApi.hpp"

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


			// Enum for the different supported rendering techniques.
			enum class RenderType
			{
				Forward, Deferred
			};


			[[nodiscard]] static auto Instance() -> Settings&;

			// Get where on the disk shaders are cached after compilation.
			// An empty location is returned if shader caching is turned off.
			[[nodiscard]] constexpr auto ShaderCacheLocation() noexcept -> const auto&;

			// Get the currently used rendering technique.
			[[nodiscard]] constexpr auto RenderingPipeline() const noexcept;

			// Get the currently used rendering API.
			[[nodiscard]] constexpr auto RenderingApi() const noexcept;

			// Get the resolution of the shadow maps used by DirectionalLights.
			// Resolutions are returned in the order of the cascades that use them.
			// More values mean more cascade splits.
			// Higher values produce better quality shadows but increase VRAM and computation costs.
			[[nodiscard]] constexpr auto DirShadowRes() -> const auto&;

			// Get the resolution of the shadows cast by SpotLights.
			// Higher values produce sharper shadows but require more VRAM.
			[[nodiscard]] constexpr auto SpotLightShadowMapResolution() const noexcept;

			// Get the resolution of the shadows cast by PointLights.
			// Higher values produce sharper shadows but require more VRAM.
			[[nodiscard]] constexpr auto PointLightShadowMapResolution() const noexcept;

			// Get the maximum number of SpotLights that will used in lighting calculations.
			// If there are more SpotLights in the scene than this number, LeopphEngine uses the ones closest to the active Camera.
			// Higher values mean more detailed lighting but can significantly reduce performance.
			[[nodiscard]] constexpr auto MaxSpotLightCount() const noexcept;

			// Get the maximum number of PointLights that will used in lighting calculations.
			// If there are more PointLights in the scene than this number, LeopphEngine uses the ones closest to the active Camera.
			// Higher values mean more detailed lighting but can significantly reduce performance.
			[[nodiscard]] constexpr auto MaxPointLightCount() const noexcept;

			// Get the correction factor when calculating shadow cascade bounds for DirectionalLights.
			[[nodiscard]] constexpr auto DirLightShadowCascadeCorrection() const noexcept;

			// Get whether shaders are cached after compilation, or recompiled during each run.
			[[nodiscard]] LEOPPHAPI auto CacheShaders() const -> bool;

			// Get whether Vsync is turned on.
			// This is exactly the same value as what Window::Vsync returns.
			[[nodiscard]] LEOPPHAPI auto Vsync() const -> bool;

			// Get the current number of shadow cascades DirectionalLights use.
			// This is the same as the size of the container set and returned by Settings::DirShadowRes.
			[[nodiscard]] LEOPPHAPI auto DirShadowCascadeCount() const -> std::size_t;

			// Set where on the disk shaders are cached after compilation.
			// Setting this property to an empty path turns shader caching off and setting a valid value turns it on.
			constexpr auto ShaderCacheLocation(std::filesystem::path path) noexcept;

			// Set the currently used rendering technique.
			// Your application must be restarted before the new value takes effect.
			constexpr auto RenderingPipeline(RenderType type) noexcept;

			// Set the currently used graphics API.
			// Your application must be restarted before the new value takes effect.
			constexpr auto RenderingApi(GraphicsApi newApi) noexcept;

			// Set the resolution of the shadows cast by PointLights.
			// Higher values produce sharper shadows but require more VRAM.
			constexpr auto PointLightShadowMapResolution(std::size_t newRes) noexcept;

			// Set the maximum number of SpotLights that will used in lighting calculations.
			// If there are more SpotLights in the scene than this number, LeopphEngine uses the ones closest to the active Camera.
			// Higher values mean more detailed lighting but can significantly reduce performance.
			constexpr auto MaxSpotLightCount(std::size_t newCount) noexcept;

			// Set the maximum number of PointLights that will used in lighting calculations.
			// If there are more PointLights in the scene than this number, LeopphEngine uses the ones closest to the active Camera.
			// Higher values mean more detailed lighting but can significantly reduce performance.
			constexpr auto MaxPointLightCount(std::size_t newCount) noexcept;

			// Set the correction factor when calculating shadow cascade bounds for DirectionalLights.
			constexpr auto DirLightShadowCascadeCorrection(float newCor) noexcept;

			// Set whether Vsync is turned on.
			// This has exactly the same effect as using Window::Vsync.
			LEOPPHAPI auto Vsync(bool value) const -> void;

			// Set the resolution of the shadow maps used by DirectionalLights.
			// Resolutions are accepted in the order of the cascades that use them.
			// More values mean more cascade splits.
			// Higher values produce better quality shadows but increase VRAM and computation costs.
			LEOPPHAPI auto DirShadowRes(std::span<const std::size_t> cascades) -> void;

			// Set the resolution of the shadows cast by SpotLights.
			// Higher values produce sharper shadows but require more VRAM.
			LEOPPHAPI auto SpotLightShadowMapResolution(std::size_t newRes) -> void;

		private:
			std::filesystem::path m_CacheLoc;
			std::vector<std::size_t> m_DirShadowRes{4096, 2048, 1024};
			std::size_t m_SpotShadowRes{2048};
			std::size_t m_PointShadowRes{1024};
			std::size_t m_NumMaxSpot{64};
			std::size_t m_NumMaxPoint{64};
			GraphicsApi m_Api{GraphicsApi::OpenGl};
			GraphicsApi m_PendingApi{m_Api};
			RenderType m_Pipeline{RenderType::Deferred};
			RenderType m_PendingPipeline{m_Pipeline};
			float m_DirShadowCascadeCorrection{.75f};
	};


	constexpr auto Settings::ShaderCacheLocation() noexcept -> const auto&
	{
		return m_CacheLoc;
	}


	constexpr auto Settings::RenderingPipeline() const noexcept
	{
		return m_Pipeline;
	}


	constexpr auto Settings::RenderingApi() const noexcept
	{
		return m_Api;
	}


	constexpr auto Settings::DirShadowRes() -> const auto&
	{
		return m_DirShadowRes;
	}


	constexpr auto Settings::SpotLightShadowMapResolution() const noexcept
	{
		return m_SpotShadowRes;
	}


	constexpr auto Settings::PointLightShadowMapResolution() const noexcept
	{
		return m_PointShadowRes;
	}


	constexpr auto Settings::MaxSpotLightCount() const noexcept
	{
		return m_NumMaxSpot;
	}


	constexpr auto Settings::MaxPointLightCount() const noexcept
	{
		return m_NumMaxPoint;
	}


	constexpr auto Settings::DirLightShadowCascadeCorrection() const noexcept
	{
		return m_DirShadowCascadeCorrection;
	}


	constexpr auto Settings::ShaderCacheLocation(std::filesystem::path path) noexcept
	{
		m_CacheLoc = std::move(path);
	}


	constexpr auto Settings::RenderingPipeline(const RenderType type) noexcept
	{
		m_PendingPipeline = type;
	}


	constexpr auto Settings::RenderingApi(const GraphicsApi newApi) noexcept
	{
		m_PendingApi = newApi;
	}


	constexpr auto Settings::PointLightShadowMapResolution(const std::size_t newRes) noexcept
	{
		m_PointShadowRes = newRes;
	}


	constexpr auto Settings::MaxSpotLightCount(const std::size_t newCount) noexcept
	{
		m_NumMaxSpot = newCount;
	}


	constexpr auto Settings::MaxPointLightCount(const std::size_t newCount) noexcept
	{
		m_NumMaxPoint = newCount;
	}


	constexpr auto Settings::DirLightShadowCascadeCorrection(const float newCor) noexcept
	{
		m_DirShadowCascadeCorrection = newCor;
	}
}
