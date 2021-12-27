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
		switch (Settings::RenderingPipeline())
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


	auto Renderer::CollectRenderables() -> const std::vector<Renderer::RenderableData>&
	{
		m_CurFrameRenderables.clear();
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
					m_CurFrameRenderables.emplace_back(renderable.get(), std::vector{std::make_pair(modelMat.Transposed(), normalMat.Transposed())}, instance->CastsShadow());
				}
			}
			if (!instMats.empty())
			{
				m_CurFrameRenderables.emplace_back(renderable.get(), instMats, instShadow);
			}
		}
		return m_CurFrameRenderables;
	}


	auto Renderer::CollectSpotLights() -> const std::vector<const SpotLight*>&
	{
		m_CurFrameSpotLights = DataManager::Instance().SpotLights();
		std::ranges::sort(m_CurFrameSpotLights, CompareLightsByDistToCam);
		if (const auto limit{Settings::MaxSpotLightCount()};
			m_CurFrameSpotLights.size() > limit)
		{
			m_CurFrameSpotLights.resize(limit);
		}
		return m_CurFrameSpotLights;
	}


	auto Renderer::CollectPointLights() -> const std::vector<const PointLight*>&
	{
		m_CurFramePointLights = DataManager::Instance().PointLights();
		std::ranges::sort(m_CurFramePointLights, CompareLightsByDistToCam);
		if (const auto limit{Settings::MaxPointLightCount()};
			m_CurFramePointLights.size() > limit)
		{
			m_CurFramePointLights.resize(limit);
		}
		return m_CurFramePointLights;
	}


	auto Renderer::CompareLightsByDistToCam(const Light* left, const Light* right) -> bool
	{
		const auto& camPosition{Camera::Active->Entity()->Transform()->Position()};
		const auto& leftDistance{Vector3::Distance(camPosition, left->Entity()->Transform()->Position())};
		const auto& rightDistance{Vector3::Distance(camPosition, right->Entity()->Transform()->Position())};
		if (std::abs(leftDistance - rightDistance) < std::numeric_limits<float>::epsilon())
		{
			return leftDistance < rightDistance;
		}
		return left < right;
	}
}
