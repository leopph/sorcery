#include "DeferredRenderer.hpp"

#include "../../components/Camera.hpp"
#include "../../config/Settings.hpp"
#include "../../data/DataManager.hpp"
#include "../../math/Matrix.hpp"
#include "../../windowing/window.h"

#include <glad/glad.h>



namespace leopph::impl
{
	DeferredRenderer::DeferredRenderer()
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glDisable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
	}


	void DeferredRenderer::Render()
	{
		/* We don't render if there is no camera to use */
		if (Camera::Active() == nullptr)
		{
			return;
		}

		const auto camViewMat{Camera::Active()->ViewMatrix()};
		const auto camProjMat{Camera::Active()->ProjectionMatrix()};

		const auto& modelsAndMats{CalcAndCollectMatrices()};
		const auto& pointLights{CollectPointLights()};
		const auto& spotLights{CollectSpotLights()};

		RenderGeometry(camViewMat, camProjMat, modelsAndMats);

		glEnable(GL_BLEND);
		RenderAmbientLight();
		RenderDirectionalLights(camViewMat, camProjMat, modelsAndMats);
		glDisable(GL_BLEND);

		RenderSkybox(camViewMat, camProjMat);
	}


	void DeferredRenderer::RenderGeometry(const Matrix4& camViewMat,
	                                      const Matrix4& camProjMat,
	                                      const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& modelsAndMats) const
	{
		m_GBuffer.Clear();
		m_GBuffer.Bind();

		m_GPassObjectShader.SetViewProjectionMatrix(camViewMat * camProjMat);
		m_GPassObjectShader.Use();

		for (const auto& [modelRes, matrices] : modelsAndMats)
		{
			modelRes->DrawShaded(m_GPassObjectShader, matrices.first, matrices.second, 0);
		}

		m_GBuffer.Unbind();
	}


	void DeferredRenderer::RenderAmbientLight() const
	{
		static_cast<void>(m_GBuffer.BindTextureForReading(m_AmbientShader, GeometryBuffer::TextureType::Ambient, 0));
		m_AmbientShader.SetAmbientLight(AmbientLight::Instance().Intensity());

		glDisable(GL_DEPTH_TEST);
		m_AmbientShader.Use();
		m_ScreenTexture.Draw();
		glEnable(GL_DEPTH_TEST);
	}


	void DeferredRenderer::RenderDirectionalLights(const Matrix4& camViewMat,
	                                               const Matrix4& camProjMat,
	                                               const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& modelsAndMats)
	{
		const auto& dirLight{DataManager::DirectionalLight()};

		if (dirLight == nullptr)
		{
			return;
		}

		const auto cameraInverseMatrix{camViewMat.Inverse()};
		const auto lightViewMatrix{Matrix4::LookAt(1000 * -dirLight->Direction(), Vector3{}, Vector3::Up())}; // TODO light pos

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

			for (const auto& [modelRes, matrices] : modelsAndMats)
			{
				modelRes->DrawDepth(matrices.first);
			}
		}

		m_DirShadowMap.UnbindTextureFromWriting();

		auto texCount{0};

		texCount = m_GBuffer.BindTextureForReading(m_DirLightShader, GeometryBuffer::TextureType::Position, texCount);
		texCount = m_GBuffer.BindTextureForReading(m_DirLightShader, GeometryBuffer::Normal, texCount);
		texCount = m_GBuffer.BindTextureForReading(m_DirLightShader, GeometryBuffer::TextureType::Diffuse, texCount);
		texCount = m_GBuffer.BindTextureForReading(m_DirLightShader, GeometryBuffer::TextureType::Specular, texCount);
		texCount = m_GBuffer.BindTextureForReading(m_DirLightShader, GeometryBuffer::TextureType::Shine, texCount);
		static_cast<void>(m_DirShadowMap.BindTexturesForReading(m_DirLightShader, texCount));

		const auto cascadeCount{Settings::CameraDirectionalShadowCascadeCount()};

		m_DirLightShader.SetDirLight(*dirLight);
		m_DirLightShader.SetCameraPosition(Camera::Active()->entity.Transform->Position());
		m_DirLightShader.SetCascadeCount(static_cast<unsigned>(cascadeCount));
		m_DirLightShader.SetLightClipMatrices(dirLightMatrices);

		static std::vector<float> cascadeFarBounds;
		cascadeFarBounds.clear();

		for (std::size_t i = 0; i < cascadeCount; ++i)
		{
			const auto viewSpaceBound{m_DirShadowMap.CascadeBounds(i)[1]};
			const Vector4 viewSpaceBoundVector{0, 0, viewSpaceBound, 1};
			const auto clipSpaceBoundVector{viewSpaceBoundVector * camProjMat};
			const auto clipSpaceBound{clipSpaceBoundVector[2]};
			cascadeFarBounds.push_back(clipSpaceBound);
		}

		m_DirLightShader.SetCascadeFarBounds(cascadeFarBounds);

		glDisable(GL_DEPTH_TEST);
		m_DirLightShader.Use();
		m_ScreenTexture.Draw();
		glEnable(GL_DEPTH_TEST);
	}


	void DeferredRenderer::RenderSkybox(const Matrix4& camViewMat, const Matrix4& camProjMat) const
	{
		const auto& window{Window::Get()};
		m_GBuffer.CopyDepthData(0, Vector2{window.Width(), window.Height()});

		if (const auto& skybox{Camera::Active()->Background().skybox}; skybox != nullptr)
		{
			m_SkyboxShader.SetUniform("viewMatrix", static_cast<Matrix4>(static_cast<Matrix3>(camViewMat)));
			m_SkyboxShader.SetUniform("projectionMatrix", camProjMat);

			m_SkyboxShader.Use();
			static_cast<SkyboxResource*>(DataManager::Find(skybox->AllFilePaths()))->Draw(m_SkyboxShader);
		}
	}
}
