#pragma once

#include "../../components/lighting/PointLight.hpp"
#include "../../components/lighting/SpotLight.hpp"
#include "../../math/Matrix.hpp"
#include "../geometry/ModelResource.hpp"

#include <memory>
#include <unordered_map>
#include <utility>
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

			Renderer& operator=(const Renderer& other) = default;
			Renderer& operator=(Renderer&& other) = default;

			virtual ~Renderer() = 0;

			virtual void Render() = 0;


		protected:
			static const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& CalcAndCollectMatrices();
			static const std::vector<const PointLight*>& CollectPointLights();
			static const std::vector<const SpotLight*>& CollectSpotLights();
	};
}
