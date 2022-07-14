#pragma once

#include "Camera.hpp"
#include "DirLight.hpp"
#include "EventReceiver.hpp"
#include "Light.hpp"
#include "MeshGroup.hpp"
#include "PointLight.hpp"
#include "SpotLight.hpp"
#include "Types.hpp"
#include "WindowEvent.hpp"
#include "events/DirShadowResEvent.hpp"
#include "events/PointShadowResEvent.hpp"
#include "events/SpotShadowResEvent.hpp"
#include "rendering/Extent.hpp"
#include "rendering/RenderObject.hpp"

#include <concepts>
#include <memory>
#include <optional>
#include <vector>


namespace leopph::internal
{
	class Renderer : public EventReceiver<WindowEvent>, public EventReceiver<DirShadowResEvent>, public EventReceiver<SpotShadowResEvent>, public EventReceiver<PointShadowResEvent>
	{
		protected:
			struct ResourceUpdateFlags
			{
				bool RenderRes : 1 = false;
				bool DirShadowRes : 1 = false;
				bool SpotShadowRes : 1 = false;
				bool PointShadowRes : 1 = false;
			};


			struct ShadowCascade;


		public:
			// Returns a renderer based on the currently set graphics api
			static auto Create() -> std::unique_ptr<Renderer>;

			// Entry point for rendering a frame
			virtual auto Render() -> void;

			// Creates a new render object encompassing the mesh group
			[[nodiscard]] virtual auto CreateRenderObject(MeshGroup const& meshGroup) -> RenderObject* = 0;

			// Destroys the render object
			virtual auto DeleteRenderObject(RenderObject* renderObject) -> void = 0;


		protected:
			/* ############################
			 * FRAME STAETE QUERY FUNCTIONS
			 * ############################ */

			[[nodiscard]] auto GetRenderRes() const -> Extent2D const&;

			[[nodiscard]] auto GetDirShadowRes() const -> std::span<u16 const>;

			[[nodiscard]] auto GetDirCorrection() const -> f32;

			[[nodiscard]] auto GetSpotShadowRes() const -> u16;

			[[nodiscard]] auto GetNumMaxSpotLights() const -> u8;

			[[nodiscard]] auto GetPointShadowRes() const -> u16;

			[[nodiscard]] auto GetNumMaxPointLights() const -> u8;

			[[nodiscard]] auto GetGamma() const -> f32;

			[[nodiscard]] auto GetAmbLight() const -> Vector3 const&;

			[[nodiscard]] auto GetDirLight() const -> std::optional<DirectionalLight const*> const&;

			[[nodiscard]] auto GetCastingSpotLights() const -> std::span<SpotLight const* const>;

			[[nodiscard]] auto GetNonCastingSpotLights() const -> std::span<SpotLight const* const>;

			[[nodiscard]] auto GetCastingPointLights() const -> std::span<PointLight const* const>;

			[[nodiscard]] auto GetNonCastingPointLights() const -> std::span<PointLight const* const>;

			[[nodiscard]] auto GetMainCamera() const -> std::optional<Camera const*> const&;


		private:
			/* #################
			 * EXTRACT FUNCTIONS
			 * ################# */

			// Extract current render resolution from global config state
			auto ExtractRenderRes() -> void;

			// Extracts current dirlight shadow map sizes from global config state
			auto ExtractDirShadowRes() -> void;

			// Extract current spotlight shadow map size from global config state
			auto ExtractSpotShadowRes() -> void;

			// Extract current pointlight shadow map size from global config state
			auto ExtractPointShadowRes() -> void;

			// Extract current dirlight shadow correction from global config state
			auto ExtractDirCorrection() -> void;

			// Extract max number of spotlights from global config state
			auto ExtractNumMaxSpotLights() -> void;

			// Extract max number of pointlights from global config state
			auto ExtractNumMaxPointLights() -> void;

			// Extract gamma from global config state
			auto ExtractGamma() -> void;

			// Extract all configuration related to rendering from global config state
			auto ExtractAllConfig() -> void;

			// Extract all active lights from global game state
			auto ExtractLights() -> void;

			// Extract main camera from global game state
			auto ExtractMainCamera() -> void;


			/* ##########################
			 * GAME STATE DATA PROCESSING
			 * ########################## */

			// Selects count lights from the vector based on distance from position.
			template<std::derived_from<Light> LightType>
			static auto SelectNearestLights(std::vector<LightType const*>& lights, Vector3 const& position, u8 count) -> void;

			// Partitions the span of lights over whether they cast shadow.
			// Sets the output span objects to the respective partitions.
			template<std::derived_from<Light> LighType>
			static auto SeparateCastingLights(std::span<LighType const*> lights, std::span<LighType const* const>& outCasting, std::span<LighType const* const>& outNonCasting) -> void;


			// This is called before every frame to make sure resources that need update are updated
			auto UpdateDependantResources() -> void;


			/* ##################################
			 * UPDATE NOTIFICATION EVENT HANDLERS
			 * ################################## */

			// Set render resolution update flag for the next frame
			auto OnEventReceived(EventReceiver<WindowEvent>::EventParamType) -> void final;

			// Set dir shadow map resolution update flag for the next frame
			auto OnEventReceived(EventReceiver<DirShadowResEvent>::EventParamType) -> void final;

			// Set spot shadow map resolution update flag for the next frame
			auto OnEventReceived(EventReceiver<SpotShadowResEvent>::EventParamType) -> void final;

			// Set point shadow map resolution update flag for the next frame
			auto OnEventReceived(EventReceiver<PointShadowResEvent>::EventParamType) -> void final;


		protected:
			/* ##################
			 * SUBCLASS CALLBACKS
			 * ################## */

			// Callback on render resolution change
			virtual auto OnRenderResChange(Extent2D renderRes) -> void = 0;

			// Callback on dir shadow map resolution change
			virtual auto OnDirShadowResChange(std::span<u16 const> resolutions) -> void = 0;

			// Callback on spot shadow map resolution change
			virtual auto OnSpotShadowResChange(u16 resolution) -> void = 0;

			// Callback on point shadow map resolution change
			virtual auto OnPointShadowResChange(u16 resolution) -> void = 0;

			// Callback on determining shadow map count requirements
			virtual auto OnDetermineShadowMapCountRequirements(u8 spot, u8 point) -> void = 0;


			/* ########################
			 * RENDER UTILITY FUNCTIONS
			 * ######################## */

			auto CalculateShadowCascades(std::vector<ShadowCascade>& out) -> void;


			/* ############
			 * RULE OF FIVE
			 * ############ */

			Renderer();

			Renderer(Renderer const& other) = default;
			auto operator=(Renderer const& other) -> Renderer& = default;

			Renderer(Renderer&& other) noexcept = default;
			auto operator=(Renderer&& other) noexcept -> Renderer& = default;

		public:
			~Renderer() noexcept override = default;


			/* ############
			 * DATA MEMBERS
			 * ############ */

		private:
			ResourceUpdateFlags m_ResUpdateFlags;
			Extent2D m_RenderRes;
			std::vector<u16> m_DirShadowRes;
			u16 m_SpotShadowRes;
			u16 m_PointShadowRes;
			f32 m_DirCorr;
			u8 m_NumMaxSpot;
			u8 m_NumMaxPoint;
			f32 m_Gamma;
			Vector3 m_AmbLight;
			std::optional<DirectionalLight const*> m_DirLight;
			std::vector<SpotLight const*> m_SpotLights;
			std::vector<PointLight const*> m_PointLights;
			std::optional<Camera const*> m_MainCamera;
			std::span<SpotLight const* const> m_CastingSpotLights;
			std::span<SpotLight const* const> m_NonCastingSpotLights;
			std::span<PointLight const* const> m_CastingPointLights;
			std::span<PointLight const* const> m_NonCastingPointLights;
	};



	struct Renderer::ShadowCascade
	{
		f32 Near;
		f32 Far;
		Matrix4 WorldToClip;
	};



	template<std::derived_from<Light> LightType>
	auto Renderer::SelectNearestLights(std::vector<LightType const*>& lights, Vector3 const& position, u8 const count) -> void
	{
		std::ranges::sort(lights, [&position](LightType const* const left, LightType const* const right) -> bool
		{
			return Vector3::Distance(position, left->Owner()->Transform()->Position()) < Vector3::Distance(position, right->Owner()->Transform()->Position());
		});

		if (lights.size() > count)
		{
			lights.resize(count);
		}
	}



	template<std::derived_from<Light> LightType>
	auto Renderer::SeparateCastingLights(std::span<LightType const*> lights, std::span<LightType const* const>& outCasting, std::span<LightType const* const>& outNonCasting) -> void
	{
		auto const itToNoCast = std::partition(std::begin(lights), std::end(lights), [](LightType const* const light)
		{
			return light->CastsShadow();
		});
		
		outCasting = {std::begin(lights), itToNoCast};
		outNonCasting = {itToNoCast, std::end(lights)};
	}
}
