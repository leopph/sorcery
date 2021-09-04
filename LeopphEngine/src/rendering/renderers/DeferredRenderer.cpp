#include "DeferredRenderer.hpp"

#include "../../components/Camera.hpp"
#include "../../config/Settings.hpp"
#include "../../data/DataManager.hpp"
#include "../../util/less/LightCloserToCamera.hpp"

#include <glad/glad.h>

#include <set>


/* DEBUG */
static unsigned int quadVAO = 0;
static unsigned int quadVBO;
static void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
/* NDEBUG */

namespace leopph::impl
{
	DeferredRenderer::DeferredRenderer() :
		m_GPassObjectShader{ Shader::Type::GPASS_OBJECT }, m_LightPassShader{ Shader::Type::LIGHTPASS }
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

		/* We collect the model matrices from the existing Models' Entities */
		CalcAndCollectMatrices();

		/* We collect the nearest pointlights */
		CollectPointLights();

		/* We collect the nearest spotlights */
		CollectSpotLights();

		DrawGeometry();
		DrawLights();
	}


	void DeferredRenderer::DrawGeometry()
	{
		m_GBuffer.Clear();
		m_GBuffer.Bind();
		m_GPassObjectShader.Use();

		m_GPassObjectShader.SetUniform("viewProjectionMatrix", m_CurrentFrameViewMatrix * m_CurrentFrameProjectionMatrix);

		for (const auto& [modelPath, matrices] : m_CurrentFrameMatrices)
		{
			static_cast<ModelResource*>(DataManager::Find(modelPath))->DrawShaded(m_GPassObjectShader, matrices.first, matrices.second, 0);
		}

		m_GBuffer.Unbind();
	}


	void DeferredRenderer::DrawLights()
	{
		m_LightPassShader.Use();

		glBindTextureUnit(0, m_GBuffer.positionTextureName);
		glBindTextureUnit(1, m_GBuffer.normalTextureName);
		glBindTextureUnit(2, m_GBuffer.ambientTextureName);
		glBindTextureUnit(3, m_GBuffer.diffuseTextureName);
		glBindTextureUnit(4, m_GBuffer.specularTextureName);
		glBindTextureUnit(5, m_GBuffer.shineTextureName);

		m_LightPassShader.SetUniform("positionTexture", 0);
		m_LightPassShader.SetUniform("normalTexture", 1);
		m_LightPassShader.SetUniform("ambientTexture", 2);
		m_LightPassShader.SetUniform("diffuseTexture", 3);
		m_LightPassShader.SetUniform("specularTexture", 4);
		m_LightPassShader.SetUniform("shineTexture", 5);

		m_LightPassShader.SetUniform("cameraPosition", Camera::Active()->entity.Transform().Position());

		/* Set up ambient light data */
		m_LightPassShader.SetUniform("ambientLight", AmbientLight::Instance().Intensity());

		/* Set up DirLight data */
		if (const auto dirLight = DataManager::DirectionalLight(); dirLight != nullptr)
		{
			m_LightPassShader.SetUniform("existsDirLight", true);
			m_LightPassShader.SetUniform("dirLight.direction", dirLight->Direction());
			m_LightPassShader.SetUniform("dirLight.diffuseColor", dirLight->Diffuse());
			m_LightPassShader.SetUniform("dirLight.specularColor", dirLight->Specular());
		}
		else
		{
			m_LightPassShader.SetUniform("existsDirLight", false);
		}

		/* Set up PointLight data */
		m_LightPassShader.SetUniform("pointLightCount", static_cast<int>(m_CurrentFrameUsedPointLights.size()));
		for (std::size_t i = 0; i < m_CurrentFrameUsedPointLights.size(); i++)
		{
			const auto& pointLight = m_CurrentFrameUsedPointLights[i];

			m_LightPassShader.SetUniform("pointLights[" + std::to_string(i) + "].position", pointLight->entity.Transform().Position());
			m_LightPassShader.SetUniform("pointLights[" + std::to_string(i) + "].diffuseColor", pointLight->Diffuse());
			m_LightPassShader.SetUniform("pointLights[" + std::to_string(i) + "].specularColor", pointLight->Specular());
			m_LightPassShader.SetUniform("pointLights[" + std::to_string(i) + "].constant", pointLight->Constant());
			m_LightPassShader.SetUniform("pointLights[" + std::to_string(i) + "].linear", pointLight->Linear());
			m_LightPassShader.SetUniform("pointLights[" + std::to_string(i) + "].quadratic", pointLight->Quadratic());
		}

		/* Set up SpotLight data */
		m_LightPassShader.SetUniform("spotLightCount", static_cast<int>(m_CurrentFrameUsedSpotLights.size()));
		for (std::size_t i = 0; i < m_CurrentFrameUsedSpotLights.size(); i++)
		{
			const auto& spotLight{ m_CurrentFrameUsedSpotLights[i] };

			m_LightPassShader.SetUniform("spotLights[" + std::to_string(i) + "].position", spotLight->entity.Transform().Position());
			m_LightPassShader.SetUniform("spotLights[" + std::to_string(i) + "].direction", spotLight->entity.Transform().Forward());
			m_LightPassShader.SetUniform("spotLights[" + std::to_string(i) + "].diffuseColor", spotLight->Diffuse());
			m_LightPassShader.SetUniform("spotLights[" + std::to_string(i) + "].specularColor", spotLight->Specular());
			m_LightPassShader.SetUniform("spotLights[" + std::to_string(i) + "].constant", spotLight->Constant());
			m_LightPassShader.SetUniform("spotLights[" + std::to_string(i) + "].linear", spotLight->Linear());
			m_LightPassShader.SetUniform("spotLights[" + std::to_string(i) + "].quadratic", spotLight->Quadratic());
			m_LightPassShader.SetUniform("spotLights[" + std::to_string(i) + "].innerAngleCosine", math::Cos(math::ToRadians(spotLight->InnerAngle())));
			m_LightPassShader.SetUniform("spotLights[" + std::to_string(i) + "].outerAngleCosine", math::Cos(math::ToRadians(spotLight->OuterAngle())));
		}

		renderQuad();
	}
}