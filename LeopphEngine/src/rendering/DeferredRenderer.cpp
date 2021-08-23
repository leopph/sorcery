#include "DeferredRenderer.hpp"

#include "../components/Camera.hpp"
#include "../config/Settings.hpp"
#include "../instances/InstanceHolder.hpp"
#include "../util/less/LightCloserToCamera.hpp"

#include <glad/glad.h>

#include <set>


namespace leopph::impl
{
	DeferredRenderer::DeferredRenderer() :
		m_GPassObjectShader{ Shader::Type::GPASS_OBJECT }
	{
		glEnable(GL_DEPTH_TEST);
	}


	void DeferredRenderer::Render()
	{
		/* We don't render if there is no camera to use */
		if (Camera::Active() == nullptr)
			return;

		/* We store the main camera's view and projection matrices for the frame */
		m_CurrentFrameViewMatrix = Camera::Active()->ViewMatrix();
		m_CurrentFrameProjectionMatrix = Camera::Active()->ProjectionMatrix();

		/* We collect the model matrices from the existing models' objects */
		CalcAndCollectModelAndNormalMatrices();

		/* We collect the nearest pointlights */
		CollectPointLights();

		/* We collect the nearest spotlights */
		CollectSpotLights();
	}


	void DeferredRenderer::CalcAndCollectModelAndNormalMatrices()
	{
		m_CurrentFrameMatrices.clear();

		for (const auto& [path, modelReference] : InstanceHolder::Models())
		{
			auto& [models, normals] = m_CurrentFrameMatrices.try_emplace(path).first->second;

			for (const auto& object : modelReference.Objects())
			{
				/* If the objet is static we query for its cached matrix */
				if (object->isStatic)
				{
					const auto& [model, normal] {InstanceHolder::ModelAndNormalMatrices(object)};
					models.emplace_back(model.Transposed());
					normals.emplace_back(normal.Transposed());
				}
				/* Otherwise we calculate it */
				else
				{
					auto modelMatrix{ Matrix4::Scale(object->Transform().Scale()) };
					modelMatrix *= static_cast<Matrix4>(object->Transform().Rotation());
					modelMatrix *= Matrix4::Translate(object->Transform().Position());
					models.emplace_back(modelMatrix.Transposed());
					normals.emplace_back(modelMatrix.Inverse());
				}
			}
		}
	}


	void DeferredRenderer::CollectPointLights()
	{
		/* This set stores lights in an ascending order based on distance from camera */
		static std::set<const PointLight*, LightCloserToCamera> allPointsLightsOrdered;

		allPointsLightsOrdered.clear();

		/* We sort the lights based on distance from camera */
		for (const PointLight* const light : InstanceHolder::PointLights())
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


	void DeferredRenderer::CollectSpotLights()
	{
		/* This set stores lights in an ascending order based on distance from camera */
		static std::set<const SpotLight*, LightCloserToCamera> allSpotLightsOrdered;

		allSpotLightsOrdered.clear();

		/* We sort the lights based on distance from camera */
		std::ranges::copy(InstanceHolder::SpotLights().begin(), InstanceHolder::SpotLights().end(), std::inserter(allSpotLightsOrdered, allSpotLightsOrdered.begin()));

		m_CurrentFrameUsedSpotLights.clear();

		/* We collect at most MAX_SPOT_LIGHTS number of them */
		const std::size_t spotLightCount{ std::min(allSpotLightsOrdered.size(), Settings::MaxSpotLightCount()) };
		std::copy_n(allSpotLightsOrdered.begin(), spotLightCount, std::back_inserter(m_CurrentFrameUsedSpotLights));
	}


	void DeferredRenderer::DrawGeometry()
	{
		m_GBuffer.Clear();
		m_GBuffer.Bind();
		m_GPassObjectShader.Use();

		for (const auto& [modelPath, matrices] : m_CurrentFrameMatrices)
		{
			InstanceHolder::GetModelReference(modelPath).DrawShaded(m_GPassObjectShader, matrices.first, matrices.second, 0);
		}

		m_GBuffer.Unbind();
	}
}