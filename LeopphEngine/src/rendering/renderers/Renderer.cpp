#include "Renderer.hpp"

#include "DeferredRenderer.hpp"
#include "ForwardRenderer.hpp"
#include "../../components/Camera.hpp"
#include "../../config/Settings.hpp"
#include "../../data/DataManager.hpp"
#include "../../util/logger.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>


namespace leopph::internal
{
	auto Renderer::Create() -> std::unique_ptr<Renderer>
	{
		switch (Settings::Instance().RenderingPipeline())
		{
			case Settings::RenderType::Forward:
				return std::make_unique<ForwardRenderer>();
			case Settings::RenderType::Deferred:
				return std::make_unique<DeferredRenderer>();
		}
		const auto errMsg{"The requested rendering pipeline is not available."};
		Logger::Instance().Critical(errMsg);
		throw std::domain_error{errMsg};
	}


	auto Renderer::CollectRenderables(std::vector<Renderer::RenderableData>& renderables) -> void
	{
		for (const auto& [renderable, instances] : DataManager::Instance().MeshGroupsAndInstances())
		{
			static std::vector<std::pair<Matrix4, Matrix4>> instMats;
			instMats.clear();
			auto instShadow{false};
			for (const auto& instance : instances)
			{
				const auto& [modelMat, normalMat]{instance->Entity()->Transform()->Matrices()};
				if (instance->Instanced())
				{
					instMats.emplace_back(modelMat.Transposed(), normalMat.Transposed());
					instShadow = instShadow || instance->CastsShadow();
				}
				else
				{
					renderables.emplace_back(renderable.get(), std::vector{std::make_pair(modelMat.Transposed(), normalMat.Transposed())}, instance->CastsShadow());
				}
			}
			if (!instMats.empty())
			{
				renderables.emplace_back(renderable.get(), instMats, instShadow);
			}
		}
	}


	auto Renderer::CollectSpotLights(std::vector<const SpotLight*>& spotLights) -> void
	{
		spotLights = DataManager::Instance().SpotLights();
		std::ranges::sort(spotLights, CompareLightsByDistToCam);
		if (const auto limit{Settings::Instance().MaxSpotLightCount()};
			spotLights.size() > limit)
		{
			spotLights.resize(limit);
		}
	}


	auto Renderer::CollectPointLights(std::vector<const PointLight*>& pointLights) -> void
	{
		pointLights = DataManager::Instance().PointLights();
		std::ranges::sort(pointLights, CompareLightsByDistToCam);
		if (const auto limit{Settings::Instance().MaxPointLightCount()};
			pointLights.size() > limit)
		{
			pointLights.resize(limit);
		}
	}


	auto Renderer::CascadeFarBoundsNdc(const Matrix4& camProjMat, const std::span<const CascadedShadowMap::CascadeBounds> cascadeBounds) -> std::span<const float>
	{
		static std::vector<float> farBounds;
		farBounds.clear();

		// Essentially we calculate (0, 0, bounds.Far, 1) * camProjMat, do a perspective divide, and take the Z component.
		std::ranges::transform(cascadeBounds, std::back_inserter(farBounds), [&](const auto& bounds)
		{
			return (bounds.Far * camProjMat[2][2] + camProjMat[3][2]) / bounds.Far; // This relies heavily on the projection matrix being row-major and projecting from a LH base to NDC.
		});

		return farBounds;
	}


	Renderer::Renderer()
	{
		Logger::Instance().Debug("Renderer created.");
	}


	auto Renderer::CompareLightsByDistToCam(const Light* left, const Light* right) -> bool
	{
		const auto& camPosition{Camera::Active()->Entity()->Transform()->Position()};
		const auto& leftDistance{Vector3::Distance(camPosition, left->Entity()->Transform()->Position())};
		const auto& rightDistance{Vector3::Distance(camPosition, right->Entity()->Transform()->Position())};
		if (std::abs(leftDistance - rightDistance) < std::numeric_limits<float>::epsilon())
		{
			return leftDistance < rightDistance;
		}
		return left < right;
	}
}
