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


	void Renderer::CalcAndCollectModelAndNormalMatrices()
	{
		m_CurrentFrameMatrices.clear();

		for (const auto& modelResource : DataManager::Models())
		{
			auto& [models, normals] = m_CurrentFrameMatrices.try_emplace(modelResource->Path).first->second;

			for (const auto& handle : DataManager::ModelComponents(modelResource))
			{
				Object& object{ static_cast<const Model*>(handle)->object };

				/* If the objet is static we query for its cached matrix */
				if (object.isStatic)
				{
					const auto& [model, normal] {DataManager::ModelAndNormalMatrices(&object)};
					models.emplace_back(model.Transposed());
					normals.emplace_back(normal.Transposed());
				}
				/* Otherwise we calculate it */
				else
				{
					auto modelMatrix{ Matrix4::Scale(object.Transform().Scale()) };
					modelMatrix *= static_cast<Matrix4>(object.Transform().Rotation());
					modelMatrix *= Matrix4::Translate(object.Transform().Position());
					models.emplace_back(modelMatrix.Transposed());
					normals.emplace_back(modelMatrix.Inverse());
				}
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