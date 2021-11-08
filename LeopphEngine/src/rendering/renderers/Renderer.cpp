#include "Renderer.hpp"

#include "DeferredRenderer.hpp"
#include "ForwardRenderer.hpp"
#include "../../components/Model.hpp"
#include "../../config/Settings.hpp"
#include "../../data/DataManager.hpp"
#include "../../util/logger.h"
#include "../../util/less/LightCloserToCamera.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <set>
#include <stdexcept>



namespace leopph::impl
{
	std::unique_ptr<Renderer> Renderer::Create()
	{
		switch (Settings::RenderingPipeline())
		{
			case Settings::RenderType::Forward:
				return std::make_unique<ForwardRenderer>();

			case Settings::RenderType::Deferred:
				return std::make_unique<DeferredRenderer>();
		}

		const auto errMsg{"There is no rendering pipeline available of the chosen type."};
		Logger::Instance().Critical(errMsg);
		throw std::domain_error{errMsg};
	}


	Renderer::~Renderer() = default;


	const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& Renderer::CalcAndCollectMatrices()
	{
		static std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>> ret;
		ret.clear();

		for (const auto& modelResource : DataManager::Models())
		{
			auto& [models, normals] = ret.try_emplace(modelResource).first->second;

			for (const auto& handle : DataManager::ModelComponents(modelResource))
			{
				const auto& transform{*static_cast<const Model*>(handle)->Entity.Transform};

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

		return ret;
	}


	const std::vector<const PointLight*>& Renderer::CollectPointLights()
	{
		/* This set stores lights in an ascending order based on distance from camera */
		static std::set<const PointLight*, LightCloserToCamera> allPointsLightsOrdered;
		static std::vector<const PointLight*> ret;

		allPointsLightsOrdered.clear();

		/* We sort the lights based on distance from camera */
		for (const PointLight* const light : DataManager::PointLights())
		{
			allPointsLightsOrdered.emplace(light);
		}

		ret.clear();

		/* We collect the first MAX_POINT_LIGHTS number of them */
		for (std::size_t count = 0; const auto& pointLight : allPointsLightsOrdered)
		{
			if (count == Settings::MaxPointLightCount())
			{
				break;
			}

			ret.push_back(pointLight);
			++count;
		}

		return ret;
	}


	const std::vector<const SpotLight*>& Renderer::CollectSpotLights()
	{
		/* This set stores lights in an ascending order based on distance from camera */
		static std::set<const SpotLight*, LightCloserToCamera> allSpotLightsOrdered;
		static std::vector<const SpotLight*> ret;

		allSpotLightsOrdered.clear();

		/* We sort the lights based on distance from camera */
		std::ranges::copy(DataManager::SpotLights().begin(), DataManager::SpotLights().end(), std::inserter(allSpotLightsOrdered, allSpotLightsOrdered.begin()));

		ret.clear();

		/* We collect at most MAX_SPOT_LIGHTS number of them */
		const auto spotLightCount{std::min(allSpotLightsOrdered.size(), Settings::MaxSpotLightCount())};
		std::copy_n(allSpotLightsOrdered.begin(), spotLightCount, std::back_inserter(ret));

		return ret;
	}
}
