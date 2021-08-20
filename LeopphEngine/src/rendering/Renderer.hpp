#pragma once

#include "shader.h"

#include "../components/lighting/PointLight.hpp"
#include "../math/matrix.h"

#include "../util/hash/PathHash.hpp"

#include <cstddef>
#include <filesystem>
#include <unordered_map>
#include <vector>


namespace leopph::impl
{
	class Renderer
	{
	public:
		Renderer();
		void Render();

	private:
		struct PointLightLess
		{
			bool operator()(const PointLight* left, const PointLight* right) const;
		};

		void CollectModelMatrices();
		void CollectPointLights();
		void RenderShadowMaps();

		Shader m_ObjectShader;
		Shader m_SkyboxShader;
		Shader m_DirectionalShadowMapShader;

		constexpr static std::size_t MAX_POINT_LIGHTS = 64;
		std::vector<const PointLight*> m_CurrentFrameUsedPointLights;

		std::unordered_map<std::filesystem::path, std::vector<Matrix4>, PathHash> m_CurrentFrameModelMatrices;
	};
}