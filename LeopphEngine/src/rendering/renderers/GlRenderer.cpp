#include "GlRenderer.hpp"

#include "GlDeferredRenderer.hpp"
#include "GlForwardRenderer.hpp"
#include "../../components/Camera.hpp"
#include "../../config/Settings.hpp"
#include "../../data/DataManager.hpp"
#include "../../util/Logger.hpp"
#include "../opengl/OpenGl.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>


namespace leopph::internal
{
	auto GlRenderer::Create() -> std::unique_ptr<GlRenderer>
	{
		switch (Settings::Instance().GetGraphicsPipeline())
		{
			case Settings::GraphicsPipeline::Forward:
				return std::make_unique<GlForwardRenderer>();
			case Settings::GraphicsPipeline::Deferred:
				return std::make_unique<GlDeferredRenderer>();
		}

		auto const errMsg{"The selected rendering pipeline is not supported for the current graphics API."};
		Logger::Instance().Critical(errMsg);
		throw std::domain_error{errMsg};
	}


	GlRenderer::GlRenderer()
	{
		opengl::Init();
	}


	auto GlRenderer::CollectRenderables(std::vector<RenderableData>& renderables) -> void
	{
		for (auto const& [meshId, groupAndInstances] : DataManager::Instance().MeshGroupsAndInstances())
		{
			static std::vector<std::pair<Matrix4, Matrix4>> instanceMatrices;

			instanceMatrices.clear();
			auto castsShadow = false;
			auto const glMeshGroup = groupAndInstances.MeshGroup.lock().get();

			for (auto const& instance : groupAndInstances.ActiveInstances)
			{
				auto const& [modelMat, normalMat]{instance->Entity()->Transform()->Matrices()};

				if (instance->Instanced())
				{
					instanceMatrices.emplace_back(modelMat.Transposed(), normalMat.Transposed());
					castsShadow = castsShadow || instance->CastsShadow();
				}
				else
				{
					renderables.emplace_back(glMeshGroup, std::vector{std::make_pair(modelMat.Transposed(), normalMat.Transposed())}, instance->CastsShadow());
				}
			}

			if (!instanceMatrices.empty())
			{
				renderables.emplace_back(glMeshGroup, instanceMatrices, castsShadow);
			}
		}
	}


	auto GlRenderer::CollectSpotLights(std::vector<SpotLight const*>& spotLights) -> void
	{
		spotLights = DataManager::Instance().ActiveSpotLights();
		std::ranges::sort(spotLights, CompareLightsByDistToCam);
		if (auto const limit{Settings::Instance().MaxSpotLightCount()};
			spotLights.size() > limit)
		{
			spotLights.resize(limit);
		}
	}


	auto GlRenderer::CollectPointLights(std::vector<PointLight const*>& pointLights) -> void
	{
		pointLights = DataManager::Instance().ActivePointLights();
		std::ranges::sort(pointLights, CompareLightsByDistToCam);
		if (auto const limit{Settings::Instance().MaxPointLightCount()};
			pointLights.size() > limit)
		{
			pointLights.resize(limit);
		}
	}


	auto GlRenderer::CascadeFarBoundsNdc(Matrix4 const& camProjMat, std::span<CascadedShadowMap::CascadeBounds const> const cascadeBounds) -> std::span<float const>
	{
		static std::vector<float> farBounds;
		farBounds.clear();

		// Essentially we calculate (0, 0, bounds.Far, 1) * camProjMat, do a perspective divide, and take the Z component.
		std::ranges::transform(cascadeBounds, std::back_inserter(farBounds), [&](auto const& bounds)
		{
			return (bounds.Far * camProjMat[2][2] + camProjMat[3][2]) / bounds.Far; // This relies heavily on the projection matrix being row-major and projecting from a LH base to NDC.
		});

		return farBounds;
	}


	auto GlRenderer::CompareLightsByDistToCam(Light const* left, Light const* right) -> bool
	{
		auto const& camPosition{Camera::Current()->Entity()->Transform()->Position()};
		auto const& leftDistance{Vector3::Distance(camPosition, left->Entity()->Transform()->Position())};
		auto const& rightDistance{Vector3::Distance(camPosition, right->Entity()->Transform()->Position())};
		if (std::abs(leftDistance - rightDistance) < std::numeric_limits<float>::epsilon())
		{
			return leftDistance < rightDistance;
		}
		return left < right;
	}
}
