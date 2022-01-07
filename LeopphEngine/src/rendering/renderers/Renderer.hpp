#pragma once

#include "../CascadedShadowMap.hpp"
#include "../../components/lighting/PointLight.hpp"
#include "../../components/lighting/SpotLight.hpp"
#include "../../math/Matrix.hpp"
#include "../geometry/GlMeshGroup.hpp"

#include <span>
#include <vector>


namespace leopph::internal
{
	class Renderer
	{
		public:
			static auto Create() -> std::unique_ptr<Renderer>;

			virtual auto Render() -> void = 0;

			virtual ~Renderer() = default;

		protected:
			struct RenderableData
			{
				const GlMeshGroup* Renderable;
				// Matrices are in the format of the rendering API.
				std::vector<std::pair<Matrix4, Matrix4>> Instances;
				bool CastsShadow;
			};


			[[nodiscard]] auto CollectRenderables() -> const std::vector<RenderableData>&;
			// Returns the first MAX_SPOT_LIGHT_COUNT SpotLights based on distance to the active Camera.
			[[nodiscard]] auto CollectSpotLights() -> const std::vector<const SpotLight*>&;
			// Returns the first MAX_POINT_LIGHT_COUNT PointLights based on distance to the active Camera.
			[[nodiscard]] auto CollectPointLights() -> const std::vector<const PointLight*>&;
			// Returns a collection of cascade far bounds in NDC.
			[[nodiscard]] static auto CascadeFarBoundsNdc(const Matrix4& camProjMat, std::span<const CascadedShadowMap::CascadeBounds> cascadeBounds) -> std::span<const float>;

			Renderer() = default;

			Renderer(const Renderer& other) = default;
			auto operator=(const Renderer& other) -> Renderer& = default;

			Renderer(Renderer&& other) = default;
			auto operator=(Renderer&& other) -> Renderer& = default;

		private:
			// Returns true if left is closer to the camera then right.
			static auto CompareLightsByDistToCam(const Light* left, const Light* right) -> bool;

			std::vector<RenderableData> m_CurFrameRenderables;
			std::vector<const SpotLight*> m_CurFrameSpotLights;
			std::vector<const PointLight*> m_CurFramePointLights;
	};
}
