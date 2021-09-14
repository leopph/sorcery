#include "DeferredRenderer.hpp"

#include "../../components/Camera.hpp"
#include "../../config/Settings.hpp"
#include "../../data/DataManager.hpp"
#include "../../math/Matrix.hpp"
#include "../../util/less/LightCloserToCamera.hpp"
#include "../../windowing/window.h"

#include <glad/glad.h>


namespace leopph::impl
{
	DeferredRenderer::DeferredRenderer() :
		m_GPassObjectShader{Shader::Type::GPASS_OBJECT},
		m_LightPassShader{Shader::Type::LIGHTPASS},
		m_SkyboxShader{Shader::Type::SKYBOX},
		m_DirShadowShader{Shader::Type::DIRECTIONAL_SHADOW_MAP},
		m_DirLightShader{Shader::Type::DIRLIGHTPASS},
		m_TextureShader{Shader::Type::TEXTURE}
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
	}


	void DeferredRenderer::Render()
	{
		/* We don't render if there is no camera to use */
		if (Camera::Active() == nullptr)
			return;

		m_CurrentFrameViewMatrix = Camera::Active()->ViewMatrix();
		m_CurrentFrameProjectionMatrix = Camera::Active()->ProjectionMatrix();

		CalcAndCollectMatrices();
		CollectPointLights();
		CollectSpotLights();

		RenderGeometry();

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		RenderDirectionalLights();
		//RenderLights();
		glDisable(GL_BLEND);

		RenderSkybox();
	}


	void DeferredRenderer::RenderGeometry()
	{
		m_GBuffer.Clear();
		m_GBuffer.Bind();

		m_GPassObjectShader.SetUniform("viewProjectionMatrix", m_CurrentFrameViewMatrix * m_CurrentFrameProjectionMatrix);

		m_GPassObjectShader.Use();

		for (const auto& [modelPath, matrices] : m_CurrentFrameMatrices)
		{
			static_cast<ModelResource*>(DataManager::Find(modelPath))->DrawShaded(m_GPassObjectShader, matrices.first, matrices.second, 0);
		}

		m_GBuffer.Unbind();
	}


	void DeferredRenderer::RenderDirectionalLights()
	{
		const auto& dirLight{DataManager::DirectionalLight()};

		if (dirLight == nullptr)
		{
			return;
		}

		const auto cameraInverseMatrix{m_CurrentFrameViewMatrix.Inverse()};
		const auto lightViewMatrix{Matrix4::LookAt(100 * -dirLight->Direction(), Vector3{}, Vector3::Up())}; // TODO light pos

		static std::vector<Matrix4> dirLightMatrices;
		dirLightMatrices.clear();

		m_DirShadowShader.Use();

		for (std::size_t i = 0; i < Settings::CameraDirectionalShadowCascadeCount(); ++i)
		{
			const auto lightWorldToClip{m_DirShadowMap.WorldToClipMatrix(i, cameraInverseMatrix, lightViewMatrix)};
			dirLightMatrices.push_back(lightWorldToClip);

			m_DirShadowMap.BindTextureForWriting(i);
			m_DirShadowMap.Clear();

			m_DirShadowShader.SetUniform("lightClipMatrix", lightWorldToClip);

			for (const auto& [modelPath, matrices] : m_CurrentFrameMatrices)
			{
				static_cast<ModelResource*>(DataManager::Find(modelPath))->DrawDepth(matrices.first);
			}

		}

		m_DirShadowMap.UnbindTextureFromWriting();

		glBindTextureUnit(0, m_GBuffer.positionTextureName);
		glBindTextureUnit(1, m_GBuffer.normalTextureName);
		glBindTextureUnit(2, m_GBuffer.diffuseTextureName);
		glBindTextureUnit(3, m_GBuffer.specularTextureName);
		glBindTextureUnit(4, m_GBuffer.shineTextureName);

		auto texCount{0};

		m_DirLightShader.SetUniform("u_PositionTexture", texCount++);
		m_DirLightShader.SetUniform("u_NormalTexture", texCount++);
		m_DirLightShader.SetUniform("u_DiffuseTexture", texCount++);
		m_DirLightShader.SetUniform("u_SpecularTexture", texCount++);
		m_DirLightShader.SetUniform("u_ShineTexture", texCount++);

		m_DirShadowMap.BindTexturesForReading(texCount);

		for (std::size_t i = 0; i < Settings::CameraDirectionalShadowCascadeCount(); i++)
		{
			m_DirLightShader.SetUniform("u_ShadowMaps[" + std::to_string(i) + "]", texCount++);
		}

		m_DirLightShader.SetUniform("u_DirLight.direction", dirLight->Direction());
		m_DirLightShader.SetUniform("u_DirLight.diffuseColor", dirLight->Diffuse());
		m_DirLightShader.SetUniform("u_DirLight.specularColor", dirLight->Specular());

		m_DirLightShader.SetUniform("u_CameraPosition", Camera::Active()->entity.Transform->Position());
		m_DirLightShader.SetUniform("u_CascadeCount", static_cast<unsigned>(Settings::CameraDirectionalShadowCascadeCount()));
		m_DirLightShader.SetUniform("u_CascadeDepth", (Camera::Active()->FarClipPlane() - Camera::Active()->NearClipPlane()) / Settings::CameraDirectionalShadowCascadeCount());

		m_DirLightShader.SetUniform("u_LightClipMatrices", dirLightMatrices);

		static std::vector<float> cascadeFarBounds;
		cascadeFarBounds.clear();

		for (std::size_t i = 0; i < Settings::CameraDirectionalShadowCascadeCount(); ++i)
		{
			const auto viewSpaceBound{m_DirShadowMap.CascadeBounds(i)[1]};
			const Vector4 viewSpaceBoundVector{0, 0, viewSpaceBound, 1};
			const auto clipSpaceBoundVector{viewSpaceBoundVector * m_CurrentFrameProjectionMatrix};
			const auto clipSpaceBound{clipSpaceBoundVector[2]};
			cascadeFarBounds.push_back(clipSpaceBound);
		}

		m_DirLightShader.SetUniform("u_CascadeFarBounds", cascadeFarBounds);

		m_DirLightShader.Use();
		m_ScreenTexture.Draw();

		/*m_TextureShader.Use();
		m_DirShadowMap.BindTexturesForReading(2);
		m_TextureShader.SetUniform("u_Texture", 2);
		m_ScreenTexture.Draw();
		m_DirShadowMap.UnbindTexturesFromReading();*/
	}


	void DeferredRenderer::RenderLights()
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

		m_LightPassShader.SetUniform("cameraPosition", Camera::Active()->entity.Transform->Position());

		/* Set up ambient light data */
		m_LightPassShader.SetUniform("ambientLight", AmbientLight::Instance().Intensity());

		/* Set up DirLight data */
		m_LightPassShader.SetUniform("existsDirLight", false);
		/*if (const auto dirLight = DataManager::DirectionalLight(); dirLight != nullptr)
		{
			m_LightPassShader.SetUniform("existsDirLight", true);
			m_LightPassShader.SetUniform("dirLight.direction", dirLight->Direction());
			m_LightPassShader.SetUniform("dirLight.diffuseColor", dirLight->Diffuse());
			m_LightPassShader.SetUniform("dirLight.specularColor", dirLight->Specular());
		}
		else
		{
			m_LightPassShader.SetUniform("existsDirLight", false);
		}*/

		/* Set up PointLight data */
		m_LightPassShader.SetUniform("pointLightCount", static_cast<int>(m_CurrentFrameUsedPointLights.size()));
		for (std::size_t i = 0; i < m_CurrentFrameUsedPointLights.size(); i++)
		{
			const auto& pointLight = m_CurrentFrameUsedPointLights[i];

			m_LightPassShader.SetUniform("pointLights[" + std::to_string(i) + "].position", pointLight->entity.Transform->Position());
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
			const auto& spotLight{m_CurrentFrameUsedSpotLights[i]};

			m_LightPassShader.SetUniform("spotLights[" + std::to_string(i) + "].position", spotLight->entity.Transform->Position());
			m_LightPassShader.SetUniform("spotLights[" + std::to_string(i) + "].direction", spotLight->entity.Transform->Forward());
			m_LightPassShader.SetUniform("spotLights[" + std::to_string(i) + "].diffuseColor", spotLight->Diffuse());
			m_LightPassShader.SetUniform("spotLights[" + std::to_string(i) + "].specularColor", spotLight->Specular());
			m_LightPassShader.SetUniform("spotLights[" + std::to_string(i) + "].constant", spotLight->Constant());
			m_LightPassShader.SetUniform("spotLights[" + std::to_string(i) + "].linear", spotLight->Linear());
			m_LightPassShader.SetUniform("spotLights[" + std::to_string(i) + "].quadratic", spotLight->Quadratic());
			m_LightPassShader.SetUniform("spotLights[" + std::to_string(i) + "].innerAngleCosine", math::Cos(math::ToRadians(spotLight->InnerAngle())));
			m_LightPassShader.SetUniform("spotLights[" + std::to_string(i) + "].outerAngleCosine", math::Cos(math::ToRadians(spotLight->OuterAngle())));
		}

		m_ScreenTexture.Draw();
	}


	void DeferredRenderer::RenderSkybox() const
	{
		const auto& window{Window::Get()};
		glBlitNamedFramebuffer(m_GBuffer.frameBufferName, 0, 0, 0, window.Width(), window.Height(), 0, 0, window.Width(), window.Height(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		if (const auto& skybox{Camera::Active()->Background().skybox}; skybox != nullptr)
		{
			m_SkyboxShader.SetUniform("viewMatrix", static_cast<Matrix4>(static_cast<Matrix3>(m_CurrentFrameViewMatrix)));
			m_SkyboxShader.SetUniform("projectionMatrix", m_CurrentFrameProjectionMatrix);

			m_SkyboxShader.Use();
			static_cast<SkyboxResource*>(DataManager::Find(skybox->AllFilePaths()))->Draw(m_SkyboxShader);
		}
	}
}