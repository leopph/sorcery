#include "DeferredRenderer.hpp"

#include "../../components/Camera.hpp"
#include "../../config/Settings.hpp"
#include "../../data/DataManager.hpp"
#include "../../math/LeopphMath.hpp"
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
		RenderSpotLights(spotLights, modelsAndMats);
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

		m_ShadowShader.Use();

		for (std::size_t i = 0; i < Settings::CameraDirectionalShadowCascadeCount(); ++i)
		{
			const auto lightWorldToClip{m_DirShadowMap.WorldToClipMatrix(i, cameraInverseMatrix, lightViewMatrix)};
			dirLightMatrices.push_back(lightWorldToClip);

			m_DirShadowMap.BindForWriting(i);
			m_DirShadowMap.Clear();

			m_ShadowShader.SetLightWorldToClipMatrix(lightWorldToClip);

			for (const auto& [modelRes, matrices] : modelsAndMats)
			{
				modelRes->DrawDepth(matrices.first);
			}
		}

		m_DirShadowMap.UnbindFromWriting();

		auto texCount{0};

		texCount = m_GBuffer.BindTextureForReading(m_DirLightShader, GeometryBuffer::TextureType::Position, texCount);
		texCount = m_GBuffer.BindTextureForReading(m_DirLightShader, GeometryBuffer::Normal, texCount);
		texCount = m_GBuffer.BindTextureForReading(m_DirLightShader, GeometryBuffer::TextureType::Diffuse, texCount);
		texCount = m_GBuffer.BindTextureForReading(m_DirLightShader, GeometryBuffer::TextureType::Specular, texCount);
		texCount = m_GBuffer.BindTextureForReading(m_DirLightShader, GeometryBuffer::TextureType::Shine, texCount);
		static_cast<void>(m_DirShadowMap.BindForReading(m_DirLightShader, texCount));

		const auto cascadeCount{Settings::CameraDirectionalShadowCascadeCount()};

		m_DirLightShader.SetDirLight(*dirLight);
		m_DirLightShader.SetCameraPosition(Camera::Active()->entity.Transform->Position());
		m_DirLightShader.SetCascadeCount(static_cast<unsigned>(cascadeCount));
		m_DirLightShader.SetLightClipMatrices(dirLightMatrices);

		static std::vector<float> cascadeFarBounds;
		cascadeFarBounds.clear();

		for (std::size_t i = 0; i < cascadeCount; ++i)
		{
			const auto viewSpaceBound{m_DirShadowMap.CascadeBoundsViewSpace(i)[1]};
			const Vector4 viewSpaceBoundVector{0, 0, viewSpaceBound, 1};
			const auto clipSpaceBoundVector{viewSpaceBoundVector * camProjMat};
			const auto clipSpaceBound{clipSpaceBoundVector[2]};
			cascadeFarBounds.push_back(clipSpaceBound);
		}

		m_DirLightShader.SetCascadeFarBounds(cascadeFarBounds);

		m_DirLightShader.Use();

		glDisable(GL_DEPTH_TEST);
		m_ScreenTexture.Draw();
		glEnable(GL_DEPTH_TEST);
	}


	void DeferredRenderer::RenderSpotLights(const std::vector<const SpotLight*>& spotLights,
	                                        const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& modelsAndMats) const
	{
		if (spotLights.empty())
		{
			return;
		}

		auto texCount{0};
		texCount = m_GBuffer.BindTextureForReading(m_SpotLightShader, GeometryBuffer::TextureType::Position, texCount);
		texCount = m_GBuffer.BindTextureForReading(m_SpotLightShader, GeometryBuffer::TextureType::Normal, texCount);
		texCount = m_GBuffer.BindTextureForReading(m_SpotLightShader, GeometryBuffer::TextureType::Diffuse, texCount);
		texCount = m_GBuffer.BindTextureForReading(m_SpotLightShader, GeometryBuffer::TextureType::Specular, texCount);
		texCount = m_GBuffer.BindTextureForReading(m_SpotLightShader, GeometryBuffer::TextureType::Shine, texCount);
		static_cast<void>(m_SpotShadowMap.BindForReading(m_SpotLightShader, texCount));

		m_SpotLightShader.SetCameraPosition(Camera::Active()->entity.Transform->Position());

		for (const auto& spotLight : spotLights)
		{
			const auto lightWorldToClipMat
			{
				Matrix4::LookAt(spotLight->entity.Transform->Position(), spotLight->entity.Transform->Position() + spotLight->entity.Transform->Forward(), Vector3::Up()) *
				Matrix4::Perspective(math::ToRadians(spotLight->OuterAngle()), 1.f, 0.001f, spotLight->Range())
			};

			m_ShadowShader.SetLightWorldToClipMatrix(lightWorldToClipMat);

			m_SpotShadowMap.BindForWriting();
			m_SpotShadowMap.Clear();

			m_ShadowShader.Use();

			for (const auto& [modelRes, matrices] : modelsAndMats)
			{
				modelRes->DrawDepth(matrices.first);
			}

			m_SpotShadowMap.UnbindFromWriting();

			m_SpotLightShader.SetSpotLight(*spotLight);
			m_SpotLightShader.SetLightClipMatrix(lightWorldToClipMat);
			m_SpotLightShader.Use();

			glDisable(GL_DEPTH_TEST);
			m_ScreenTexture.Draw();
			glEnable(GL_DEPTH_TEST);
		}
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
