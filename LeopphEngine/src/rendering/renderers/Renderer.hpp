#pragma once

#include "../../components/lighting/PointLight.hpp"
#include "../../components/lighting/SpotLight.hpp"
#include "../../math/Matrix.hpp"
#include "../geometry/GlMeshGroup.hpp"

#include <vector>


namespace leopph::impl
{
	class Renderer
	{
		public:
			static std::unique_ptr<Renderer> Create();

			Renderer() = default;
			Renderer(const Renderer& other) = default;
			Renderer(Renderer&& other) = default;

			virtual ~Renderer() = 0;

			Renderer& operator=(const Renderer& other) = default;
			Renderer& operator=(Renderer&& other) = default;

			virtual void Render() = 0;

		protected:
			struct RenderableData
			{
				GlMeshGroup Renderable;
				// Matrices are in the format of the rendering API.
				std::vector<std::pair<Matrix4, Matrix4>> Instances;
				bool CastsShadow;
			};


			static const std::vector<RenderableData>& CollectRenderables();
			static const std::vector<const PointLight*>& CollectPointLights();
			static const std::vector<const SpotLight*>& CollectSpotLights();
	};
}
