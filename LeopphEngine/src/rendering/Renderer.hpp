#pragma once

#include "Shader.hpp"

#include "../components/lighting/PointLight.hpp"
#include "../components/lighting/SpotLight.hpp"
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

		constexpr static std::size_t MAX_POINT_LIGHTS = 64;
		std::vector<const PointLight*> m_CurrentFrameUsedPointLights;

		constexpr static std::size_t MAX_SPOT_LIGHTS = 64;
		std::vector<const SpotLight*> m_CurrentFrameUsedSpotLights;

		std::unordered_map<std::filesystem::path, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>, PathHash> m_CurrentFrameMatrices;

		inline const static Vector2 SHADOW_MAP_RESOLUTION{ 1024, 1024 };

		Matrix4 m_CurrentFrameViewMatrix;
		Matrix4 m_CurrentFrameProjectionMatrix;
		Matrix4 m_CurrentFrameDirectionalTransformMatrix;
	};
}