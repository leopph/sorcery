#pragma once

#include "Shader.hpp"

#include "../components/lighting/PointLight.hpp"
#include "../math/Matrix.hpp"

#include "../util/hash/PathHash.hpp"

#include <cstddef>
#include <filesystem>
#include <unordered_map>
#include <utility>
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

		void CalcAndCollectModelAndNormalMatrices();
		void CollectPointLights();
		void RenderDirectionalShadowMap() const;
		void RenderPointShadowMaps();
		void RenderShadedObjects();
		void RenderSkybox() const;

		Shader m_ObjectShader;
		Shader m_SkyboxShader;
		Shader m_DirectionalShadowMapShader;

		constexpr static std::size_t MAX_POINT_LIGHTS = 64;
		std::vector<const PointLight*> m_CurrentFrameUsedPointLights;

		std::unordered_map<std::filesystem::path, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>, PathHash> m_CurrentFrameModelMatrices;

		inline const static Vector2 SHADOW_MAP_RESOLUTION{ 1024, 1024 };

		Matrix4 m_CurrentFrameViewMatrix;
		Matrix4 m_CurrentFrameProjectionMatrix;
	};
}