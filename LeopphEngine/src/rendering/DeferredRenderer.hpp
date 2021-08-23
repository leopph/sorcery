#pragma once

#include "../components/lighting/PointLight.hpp"
#include "../components/lighting/SpotLight.hpp"
#include "../math/Matrix.hpp"
#include "buffers/GeometryBuffer.hpp"
#include "Shader.hpp"

#include "../util/hash/PathHash.hpp"

#include <filesystem>
#include <unordered_map>
#include <vector>


namespace leopph::impl
{
	class DeferredRenderer
	{
	public:
		DeferredRenderer();

		DeferredRenderer(const DeferredRenderer& other) = default;
		DeferredRenderer(DeferredRenderer&& other) = default;

		DeferredRenderer& operator=(const DeferredRenderer& other) = default;
		DeferredRenderer& operator=(DeferredRenderer&& other) = default;

		~DeferredRenderer() = default;

		void Render();

	private:
		void CalcAndCollectModelAndNormalMatrices();
		void CollectPointLights();
		void CollectSpotLights();
		void DrawGeometry();

		std::vector<const PointLight*> m_CurrentFrameUsedPointLights;
		std::vector<const SpotLight*> m_CurrentFrameUsedSpotLights;

		std::unordered_map<std::filesystem::path, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>, PathHash> m_CurrentFrameMatrices;

		Matrix4 m_CurrentFrameViewMatrix;
		Matrix4 m_CurrentFrameProjectionMatrix;

		GeometryBuffer m_GBuffer;

		Shader m_GPassObjectShader;
	};
}