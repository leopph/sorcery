#pragma once

#include "AmbientLight.hpp"
#include "DirLight.hpp"
#include "Matrix.hpp"
#include "PointLight.hpp"
#include "Renderer.hpp"
#include "SpotLight.hpp"
#include "../CascadedShadowMap.hpp"
#include "../geometry/GlMeshGroup.hpp"

#include <span>
#include <vector>


namespace leopph::internal
{
	class GlRenderer : public Renderer
	{
		public:
			// Constructs an OpenGL renderer for the selected rendering pipeline.
			static auto Create() -> std::unique_ptr<GlRenderer>;

		protected:
			GlRenderer();

		public:
			GlRenderer(GlRenderer const& other) = delete;
			auto operator=(GlRenderer const& other) -> GlRenderer& = delete;

			GlRenderer(GlRenderer&& other) noexcept = delete;
			auto operator=(GlRenderer&& other) noexcept -> GlRenderer& = delete;

			~GlRenderer() override = default;

		protected:
			struct RenderableData
			{
				GlMeshGroup* Renderable;
				// Matrices are in the format of the rendering API.
				std::vector<std::pair<Matrix4, Matrix4>> Instances;
				bool CastsShadow;
			};


			struct ShadowCount
			{
				bool Directional{false};
				std::size_t Spot{0};
				std::size_t Point{0};
			};


			static auto CollectRenderables(std::vector<RenderableData>& renderables) -> void;
			// Places the first MAX_SPOT_LIGHT_COUNT SpotLights based on distance to the active Camera into the passed vector.
			static auto CollectSpotLights(std::vector<SpotLight const*>& spotLights) -> void;
			// Places the first MAX_POINT_LIGHT_COUNT PointLights based on distance to the active Camera into the passed vector.
			static auto CollectPointLights(std::vector<PointLight const*>& pointLights) -> void;
			// Returns a collection of cascade far bounds in NDC.
			[[nodiscard]] static auto CascadeFarBoundsNdc(Matrix4 const& camProjMat, std::span<CascadedShadowMap::CascadeBounds const> cascadeBounds) -> std::span<float const>;

			static auto SetAmbientData(AmbientLight const& light, ShaderProgram& lightShader) -> void;
			static auto SetDirectionalData(DirectionalLight const* dirLight, ShaderProgram& shader) -> void;
			static auto SetSpotData(std::span<SpotLight const* const> spotLights, ShaderProgram& shader) -> void;
			static auto SetSpotDataIgnoreShadow(std::span<SpotLight const* const> spotLights, ShaderProgram& shader) -> void;
			static auto SetPointData(std::span<PointLight const* const> pointLights, ShaderProgram& shader) -> void;
			static auto SetPointDataIgnoreShadow(std::span<PointLight const* const> pointLights, ShaderProgram& shader) -> void;
			static auto CountShadows(DirectionalLight const* dirLight, std::span<SpotLight const* const> spotLights, std::span<PointLight const* const> pointLights) -> ShadowCount;

		private:
			// Returns true if left is closer to the camera then right.
			static auto CompareLightsByDistToCam(Light const* left, Light const* right) -> bool;
	};
}
