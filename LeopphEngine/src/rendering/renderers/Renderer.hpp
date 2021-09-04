#pragma once

#include "../../components/lighting/PointLight.hpp"
#include "../../components/lighting/SpotLight.hpp"
#include "../../math/Matrix.hpp"
#include "../../util/hash/PathHash.hpp"

#include <filesystem>
#include <unordered_map>
#include <vector>


namespace leopph::impl
{
	class Renderer
	{
	public:
		Renderer() = default;
		Renderer(const Renderer& other) = default;
		Renderer(Renderer&& other) = default;
		
		Renderer& operator=(const Renderer& other) = default;
		Renderer& operator=(Renderer&& other) = default;

		virtual ~Renderer() = 0;

		virtual void Render() = 0;


	protected:
		void CalcAndCollectMatrices();
		void CollectPointLights();
		void CollectSpotLights();

		std::vector<const PointLight*> m_CurrentFrameUsedPointLights;
		std::vector<const SpotLight*> m_CurrentFrameUsedSpotLights;

		Matrix4 m_CurrentFrameViewMatrix;
		Matrix4 m_CurrentFrameProjectionMatrix;

		std::unordered_map<std::filesystem::path, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>, PathHash> m_CurrentFrameMatrices;
	};
}