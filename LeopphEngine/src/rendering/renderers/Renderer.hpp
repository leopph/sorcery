#pragma once

#include "../../components/lighting/PointLight.hpp"
#include "../../components/lighting/SpotLight.hpp"
#include "../../math/Matrix.hpp"
#include "../geometry/GlMeshGroup.hpp"

#include <vector>


namespace leopph::internal
{
	class Renderer
	{
		public:
			static auto Create() -> std::unique_ptr<Renderer>;

			Renderer() = default;
			Renderer(const Renderer& other) = default;
			Renderer(Renderer&& other) = default;

			virtual ~Renderer() = 0;

			auto operator=(const Renderer& other) -> Renderer& = default;
			auto operator=(Renderer&& other) -> Renderer& = default;

			virtual auto Render() -> void = 0;

		protected:
			struct RenderableData
			{
				GlMeshGroup Renderable;
				// Matrices are in the format of the rendering API.
				std::vector<std::pair<Matrix4, Matrix4>> Instances;
				bool CastsShadow;
			};


			auto CollectRenderables() -> const std::vector<RenderableData>&;
			static auto CollectPointLights() -> const std::vector<const PointLight*>&;
			static auto CollectSpotLights() -> const std::vector<const SpotLight*>&;

		private:
			std::vector<RenderableData> m_CurFrameRenderables;
	};
}
