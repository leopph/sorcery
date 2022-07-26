#pragma once

#include "Camera.hpp"
#include "DirLight.hpp"
#include "EventReceiver.hpp"
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
		struct ResourceUpdateFlags
		{
			bool renderRes : 1 = false;
			bool dirShadowRes : 1 = false;
			bool spotShadowRes : 1 = false;
			bool pointShadowRes : 1 = false;
		};


		struct ShadowCascade
		{
			f32 near;
			f32 far;
			Matrix4 wordToClip;
		};


		struct DirLightUboData
		{
			Vector3 direction;
			alignas(16) Vector3 diffuse;
			alignas(16) Vector3 specular;
		};


		struct SpotLightUboData
		{
			Vector3 position;
			alignas(16) Vector3 direction;
			alignas(16) Vector3 diffuse;
			alignas(16) Vector3 specular;
			f32 range;
			f32 innerCos;
			f32 outerCos;
		};


		struct PointLightUboData
		{
			Vector3 position;
			alignas(16) Vector3 diffuse;
			alignas(16) Vector3 specular;
			f32 range;
		};
		#pragma warning(pop)


		struct ShadowCascadeData
		{
			f32 nearClip;
			f32 correction;
			std::vector<u16> res;
		};


		struct DirLightData
		{
			Vector3 direction;
			Vector3 diffuse;
			Vector3 specular;
			std::optional<ShadowCascadeData> cascades;
		};


		struct SpotLightData
		{
			Vector3 position;
			Vector3 direction;
			Vector3 diffuse;
			Vector3 specular;
			f32 range;
			f32 innerCos;
			f32 outerCos;
		};


		struct PointLightData
		{
			Vector3 diffuse;
			Vector3 specular;
			f32 range;
		};


		struct CameraTransformData
		{
			Matrix4 viewMat;
			Matrix4 viewMatInv;
			Matrix4 projMat;
			Matrix4 projMatInv;
			Matrix4 viewProjMat;
			Matrix4 viewProjMatInv;
		};


		struct CameraData
		{
			Vector3 position;
			Vector3 pos;
			CameraTransformData transformData;
		};


		struct ScreenData
		{
			u32 width;
			u32 height;
			f32 gamma;
		};


		public:
			// Entry point for rendering a frame
			void render();

			// Creates a new render object encompassing the mesh group
			[[nodiscard]] virtual RenderObject* create_render_object(MeshGroup const& meshGroup) = 0;

			// Destroys the render object
			virtual void delete_render_object(RenderObject* renderObject) = 0;


		private:
			void extract_spot_shadow_res();
			void extract_point_shadow_res();
			void extract_num_max_spot_lights();
			void extract_num_max_point_lights();
			void extract_all_config();
			void extract_all_lights();


			// Extract all information from global game and config states
			// Returns true if rendering should continue based on the observed state, false if it should abort.
			[[nodiscard]] bool extract();
			void prepare();
			void draw();


			// Selects count lights from the vector based on distance from position.
			template<std::derived_from<Light> LightType>
			static void select_nearest_lights(std::vector<LightType const*>& lights, Vector3 const& position, u8 count);

			// Partitions the span of lights over whether they cast shadow.
			// Sets the output span objects to the respective partitions.
			template<std::derived_from<Light> LighType>
			static void separate_casting_lights(std::span<LighType const*> lights, std::span<LighType const* const>& outCasting, std::span<LighType const* const>& outNonCasting);

			// Prepares all lighting data for later usage.
			void prepare_all_lighting_data();


			void update_dependant_resources();

			void forward_render();
			void deferred_render();


			void OnEventReceived(EventReceiver<WindowEvent>::EventParamType) override;
			void OnEventReceived(EventReceiver<DirShadowResEvent>::EventParamType) override;
			void OnEventReceived(EventReceiver<SpotShadowResEvent>::EventParamType) override;
			void OnEventReceived(EventReceiver<PointShadowResEvent>::EventParamType) override;


			virtual void on_render_res_change(Vector2U renderRes) = 0;
			virtual void on_dir_shadow_res_change(std::span<u16 const> resolutions) = 0;
			virtual void on_spot_shadow_res_change(u16 resolution) = 0;
			virtual void on_point_shadow_res_change(u16 resolution) = 0;
			virtual void on_determine_shadow_map_count_requirements(u8 spot, u8 point) = 0;


			void calculate_shadow_cascades(std::vector<ShadowCascade>& out);


		public:
			Renderer();

			Renderer(Renderer const& other) = delete;
			Renderer& operator=(Renderer const& other) = delete;

			Renderer(Renderer&& other) noexcept = delete;
			Renderer& operator=(Renderer&& other) noexcept = delete;

			~Renderer() noexcept override = default;


		private:
			CameraData mCamData;
			ScreenData mScreenData;
			std::optional<DirLightData> mDirData;


			ResourceUpdateFlags mResUpdateFlags;
			u16 mSpotShadowRes;
			u16 mPointShadowRes;
			Vector3 mAmbLight;
			std::optional<Camera const*> mMainCam;

			std::vector<SpotLight const*> mSpotLights;
			std::vector<PointLight const*> mPointLights;
			std::span<SpotLight const* const> mCastingSpots;
			std::span<SpotLight const* const> mNonCastingSpots;
			std::span<PointLight const* const> mCastingPoints;
			std::span<PointLight const* const> mNonCastingPoints;
	};



	template<std::derived_from<Light> LightType>
	void Renderer::select_nearest_lights(std::vector<LightType const*>& lights, Vector3 const& position, u8 const count)
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
	void Renderer::separate_casting_lights(std::span<LightType const*> lights, std::span<LightType const* const>& outCasting, std::span<LightType const* const>& outNonCasting)
	{
		auto const itToNoCast = std::partition(std::begin(lights), std::end(lights), [](LightType const* const light)
		{
			return light->is_casting_shadow();
		});

		outCasting = {std::begin(lights), itToNoCast};
		outNonCasting = {itToNoCast, std::end(lights)};
	}
}
