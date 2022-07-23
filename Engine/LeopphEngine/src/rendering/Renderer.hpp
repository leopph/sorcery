#pragma once

#include "Camera.hpp"
#include "DirLight.hpp"
#include "EventReceiver.hpp"
#include "Extent.hpp"
#include "Light.hpp"
#include "MeshGroup.hpp"
#include "PointLight.hpp"
#include "RenderObject.hpp"
#include "SpotLight.hpp"
#include "Types.hpp"
#include "WindowEvent.hpp"
#include "../events/DirShadowResEvent.hpp"
#include "../events/PointShadowResEvent.hpp"
#include "../events/SpotShadowResEvent.hpp"

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
			static std::unique_ptr<Renderer> Create();

			// Entry point for rendering a frame
			virtual void Render();

			// Creates a new render object encompassing the mesh group
			[[nodiscard]] virtual RenderObject* CreateRenderObject(MeshGroup const& meshGroup) = 0;

			// Destroys the render object
			virtual void DeleteRenderObject(RenderObject* renderObject) = 0;


		protected:
			/* ############################
			 * FRAME STAETE QUERY FUNCTIONS
			 * ############################ */

			[[nodiscard]] Extent2D const& GetRenderRes() const;

			[[nodiscard]] std::span<u16 const> GetDirShadowRes() const;

			[[nodiscard]] f32 GetDirCorrection() const;

			[[nodiscard]] u16 GetSpotShadowRes() const;

			[[nodiscard]] u8 GetNumMaxSpotLights() const;

			[[nodiscard]] u16 GetPointShadowRes() const;

			[[nodiscard]] u8 GetNumMaxPointLights() const;

			[[nodiscard]] f32 GetGamma() const;

			[[nodiscard]] Vector3 const& GetAmbLight() const;

			[[nodiscard]] std::optional<DirectionalLight const*> const& GetDirLight() const;

			[[nodiscard]] std::span<SpotLight const* const> GetCastingSpotLights() const;

			[[nodiscard]] std::span<SpotLight const* const> GetNonCastingSpotLights() const;

			[[nodiscard]] std::span<PointLight const* const> GetCastingPointLights() const;

			[[nodiscard]] std::span<PointLight const* const> GetNonCastingPointLights() const;

			[[nodiscard]] std::optional<Camera const*> const& GetMainCamera() const;


		private:
			/* #################
			 * EXTRACT FUNCTIONS
			 * ################# */

			// Extract current render resolution from global config state
			void ExtractRenderRes();

			// Extracts current dirlight shadow map sizes from global config state
			void ExtractDirShadowRes();

			// Extract current spotlight shadow map size from global config state
			void ExtractSpotShadowRes();

			// Extract current pointlight shadow map size from global config state
			void ExtractPointShadowRes();

			// Extract current dirlight shadow correction from global config state
			void ExtractDirCorrection();

			// Extract max number of spotlights from global config state
			void ExtractNumMaxSpotLights();

			// Extract max number of pointlights from global config state
			void ExtractNumMaxPointLights();

			// Extract gamma from global config state
			void ExtractGamma();

			// Extract all configuration related to rendering from global config state
			void ExtractAllConfig();

			// Extract all active lights from global game state
			void ExtractLights();

			// Extract main camera from global game state
			void ExtractMainCamera();


			/* ##########################
			 * GAME STATE DATA PROCESSING
			 * ########################## */

			// Selects count lights from the vector based on distance from position.
			template<std::derived_from<Light> LightType>
			static void SelectNearestLights(std::vector<LightType const*>& lights, Vector3 const& position, u8 count);

			// Partitions the span of lights over whether they cast shadow.
			// Sets the output span objects to the respective partitions.
			template<std::derived_from<Light> LighType>
			static void SeparateCastingLights(std::span<LighType const*> lights, std::span<LighType const* const>& outCasting, std::span<LighType const* const>& outNonCasting);


			// This is called before every frame to make sure resources that need update are updated
			void UpdateDependantResources();


			/* ##################################
			 * UPDATE NOTIFICATION EVENT HANDLERS
			 * ################################## */

			// Set render resolution update flag for the next frame
			void OnEventReceived(EventReceiver<WindowEvent>::EventParamType) final;

			// Set dir shadow map resolution update flag for the next frame
			void OnEventReceived(EventReceiver<DirShadowResEvent>::EventParamType) final;

			// Set spot shadow map resolution update flag for the next frame
			void OnEventReceived(EventReceiver<SpotShadowResEvent>::EventParamType) final;

			// Set point shadow map resolution update flag for the next frame
			void OnEventReceived(EventReceiver<PointShadowResEvent>::EventParamType) final;


		protected:
			/* ##################
			 * SUBCLASS CALLBACKS
			 * ################## */

			// Callback on render resolution change
			virtual void OnRenderResChange(Extent2D renderRes) = 0;

			// Callback on dir shadow map resolution change
			virtual void OnDirShadowResChange(std::span<u16 const> resolutions) = 0;

			// Callback on spot shadow map resolution change
			virtual void OnSpotShadowResChange(u16 resolution) = 0;

			// Callback on point shadow map resolution change
			virtual void OnPointShadowResChange(u16 resolution) = 0;

			// Callback on determining shadow map count requirements
			virtual void OnDetermineShadowMapCountRequirements(u8 spot, u8 point) = 0;


			/* ########################
			 * RENDER UTILITY FUNCTIONS
			 * ######################## */

			void CalculateShadowCascades(std::vector<ShadowCascade>& out);


			/* ############
			 * RULE OF FIVE
			 * ############ */

			Renderer();

			Renderer(Renderer const& other) = default;
			Renderer& operator=(Renderer const& other) = default;

			Renderer(Renderer&& other) noexcept = default;
			Renderer& operator=(Renderer&& other) noexcept = default;

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
	void Renderer::SelectNearestLights(std::vector<LightType const*>& lights, Vector3 const& position, u8 const count)
	{
		std::ranges::sort(lights, [&position](LightType const* const left, LightType const* const right) -> bool
		{
			return Vector3::Distance(position, left->Owner()->get_transform().get_position()) < Vector3::Distance(position, right->Owner()->get_transform().get_position());
		});

		if (lights.size() > count)
		{
			lights.resize(count);
		}
	}



	template<std::derived_from<Light> LightType>
	void Renderer::SeparateCastingLights(std::span<LightType const*> lights, std::span<LightType const* const>& outCasting, std::span<LightType const* const>& outNonCasting)
	{
		auto const itToNoCast = std::partition(std::begin(lights), std::end(lights), [](LightType const* const light)
		{
			return light->CastsShadow();
		});

		outCasting = {std::begin(lights), itToNoCast};
		outNonCasting = {itToNoCast, std::end(lights)};
	}
}
