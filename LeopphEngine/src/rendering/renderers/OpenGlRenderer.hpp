#pragma once

#include "Renderer.hpp"
#include "../CascadedShadowMap.hpp"
#include "../../components/lighting/PointLight.hpp"
#include "../../components/lighting/SpotLight.hpp"
#include "../../math/Matrix.hpp"
#include "../geometry/GlMeshGroup.hpp"

#include <span>
#include <vector>


namespace leopph::internal
{
	class OpenGlRenderer : public Renderer
	{
		public:
			// Constructs an OpenGL renderer for the selected rendering pipeline.
			static auto Create() -> std::unique_ptr<OpenGlRenderer>;

			OpenGlRenderer(const OpenGlRenderer& other) = delete;
			auto operator=(const OpenGlRenderer& other) -> OpenGlRenderer& = delete;

			OpenGlRenderer(OpenGlRenderer&& other) noexcept = delete;
			auto operator=(OpenGlRenderer&& other) noexcept -> OpenGlRenderer& = delete;

			~OpenGlRenderer() override = default;

		protected:
			OpenGlRenderer();


			struct RenderableData
			{
				const GlMeshGroup* Renderable;
				// Matrices are in the format of the rendering API.
				std::vector<std::pair<Matrix4, Matrix4>> Instances;
				bool CastsShadow;
			};


			static auto CollectRenderables(std::vector<RenderableData>& renderables) -> void;
			// Places the first MAX_SPOT_LIGHT_COUNT SpotLights based on distance to the active Camera into the passed vector.
			static auto CollectSpotLights(std::vector<const SpotLight*>& spotLights) -> void;
			// Places the first MAX_POINT_LIGHT_COUNT PointLights based on distance to the active Camera into the passed vector.
			static auto CollectPointLights(std::vector<const PointLight*>& pointLights) -> void;
			// Returns a collection of cascade far bounds in NDC.
			[[nodiscard]] static auto CascadeFarBoundsNdc(const Matrix4& camProjMat, std::span<const CascadedShadowMap::CascadeBounds> cascadeBounds) -> std::span<const float>;

		private:
			// Returns true if left is closer to the camera then right.
			static auto CompareLightsByDistToCam(const Light* left, const Light* right) -> bool;
	};
}
