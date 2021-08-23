#pragma once

#include "Shader.hpp"

#include "../components/lighting/PointLight.hpp"
#include "../components/lighting/SpotLight.hpp"
#include "../math/Matrix.hpp"

#include "../util/hash/PathHash.hpp"

#include <filesystem>
#include <unordered_map>
#include <utility>
#include <vector>


namespace leopph::impl
{
	class ForwardRenderer
	{
	public:
		ForwardRenderer();
		void Render();

	private:
		struct LightLess
		{
			bool operator()(const Light* left, const Light* right) const;
		};

		void CalcAndCollectModelAndNormalMatrices();
		void CollectPointLights();
		void CollectSpotLights();
		void RenderDirectionalShadowMap();
		void RenderPointShadowMaps();
		void RenderShadedObjects();
		void RenderSkybox() const;

		Shader m_ObjectShader;
		Shader m_SkyboxShader;
		Shader m_DirectionalShadowMapShader;
		
		std::vector<const PointLight*> m_CurrentFrameUsedPointLights;
		std::vector<const SpotLight*> m_CurrentFrameUsedSpotLights;

		std::unordered_map<std::filesystem::path, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>, PathHash> m_CurrentFrameMatrices;

		Matrix4 m_CurrentFrameViewMatrix;
		Matrix4 m_CurrentFrameProjectionMatrix;
		Matrix4 m_CurrentFrameDirectionalTransformMatrix;
	};
}