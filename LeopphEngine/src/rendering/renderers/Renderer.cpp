#include "Renderer.hpp"

#include "../../data/DataManager.hpp"
#include "../../components/Model.hpp"
#include "../../config/Settings.hpp"
#include "../../util/less/LightCloserToCamera.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <set>


namespace leopph::impl
{
	Renderer::~Renderer() = default;


	void Renderer::CalcAndCollectMatrices()
	{
		m_CurrentFrameMatrices.clear();

		for (const auto& modelResource : DataManager::Models())
		{
			auto& [models, normals] = m_CurrentFrameMatrices.try_emplace(modelResource->Path).first->second;

			for (const auto& handle : DataManager::ModelComponents(modelResource))
			{
				const auto& transform{ static_cast<const Model*>(handle)->object.Transform() };

				if (transform.WasAltered)
				{
					auto modelMatrix{Matrix4::Scale(transform.Scale())};
					modelMatrix *= static_cast<Matrix4>(transform.Rotation());
					modelMatrix *= Matrix4::Translate(transform.Position());

					/* OpenGL accepts our matrices transposed,
					 * that's why this is done this way. Weird, eh? */
					DataManager::StoreMatrices(&transform, modelMatrix.Transposed(), modelMatrix.Inverse());
				}

				const auto& [model, normal]{DataManager::GetMatrices(&transform)};
				models.emplace_back(model);
				normals.emplace_back(normal);
			}
		}
	}


	void Renderer::CollectPointLights()
	{
		/* This set stores lights in an ascending order based on distance from camera */
		static std::set<const PointLight*, LightCloserToCamera> allPointsLightsOrdered;

		allPointsLightsOrdered.clear();

		/* We sort the lights based on distance from camera */
		for (const PointLight* const light : DataManager::PointLights())
		{
			allPointsLightsOrdered.emplace(light);
		}

		m_CurrentFrameUsedPointLights.clear();

		/* We collect the first MAX_POINT_LIGHTS number of them */
		for (std::size_t count = 0; const auto & pointLight : allPointsLightsOrdered)
		{
			if (count == Settings::MaxPointLightCount())
			{
				break;
			}

			m_CurrentFrameUsedPointLights.emplace_back(pointLight);
			++count;
		}
	}


	void Renderer::CollectSpotLights()
	{
		/* This set stores lights in an ascending order based on distance from camera */
		static std::set<const SpotLight*, LightCloserToCamera> allSpotLightsOrdered;

		allSpotLightsOrdered.clear();

		/* We sort the lights based on distance from camera */
		std::ranges::copy(DataManager::SpotLights().begin(), DataManager::SpotLights().end(), std::inserter(allSpotLightsOrdered, allSpotLightsOrdered.begin()));

		m_CurrentFrameUsedSpotLights.clear();

		/* We collect at most MAX_SPOT_LIGHTS number of them */
		const std::size_t spotLightCount{ std::min(allSpotLightsOrdered.size(), Settings::MaxSpotLightCount()) };
		std::copy_n(allSpotLightsOrdered.begin(), spotLightCount, std::back_inserter(m_CurrentFrameUsedSpotLights));
	}
}