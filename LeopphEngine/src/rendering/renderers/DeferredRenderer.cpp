#include "DeferredRenderer.hpp"

#include "../../components/Camera.hpp"
#include "../../components/lighting/AmbientLight.hpp"
#include "../../config/Settings.hpp"
#include "../../data/DataManager.hpp"
#include "../../math/LeopphMath.hpp"
#include "../../math/Matrix.hpp"
#include "../../windowing/Window.hpp"

#include <glad/glad.h>



namespace leopph::impl
{
	DeferredRenderer::DeferredRenderer() :
		m_TextureShader{
			{
				{ShaderProgram::LightPassVertSrc, ShaderType::Vertex, {}},
				{ShaderProgram::TextureFragSrc, ShaderType::Fragment, {}}
			}
		},
		m_ShadowShader{
			{
				{ShaderProgram::ShadowMapVertSrc, ShaderType::Vertex, {}}
			}
		},
		m_GPassObjectShader{
			{
				{ShaderProgram::GPassObjectVertSrc, ShaderType::Vertex, {}},
				{ShaderProgram::GPassObjectFragSrc, ShaderType::Fragment, {}}
			}
		},
		m_AmbientShader{
			{
				{ShaderProgram::LightPassVertSrc, ShaderType::Vertex, {}},
				{ShaderProgram::AmbLightFragSrc, ShaderType::Fragment, {}}
			}
		},
		m_ShadowedDirLightShader{
			{
				{ShaderProgram::LightPassVertSrc, ShaderType::Vertex, {}},
				{ShaderProgram::DirLightPassFragSrc, ShaderType::Fragment, {"CAST_SHADOW"}}
			}
		},
		m_UnshadowedDirLightShader{
			{
				{ShaderProgram::LightPassVertSrc, ShaderType::Vertex, {}},
				{ShaderProgram::DirLightPassFragSrc, ShaderType::Fragment, {}}
			}
		},
		m_SpotLightShader{
			{
				{ShaderProgram::LightPassVertSrc, ShaderType::Vertex, {}},
				{ShaderProgram::SpotLightPassFragSrc, ShaderType::Fragment, {}}
			}
		},
		m_SkyboxShader{
			{
				{ShaderProgram::SkyboxVertSrc, ShaderType::Vertex, {}},
				{ShaderProgram::SkyboxFragSrc, ShaderType::Fragment, {}}
			}
		}
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

		RenderAmbientLight();
		glEnable(GL_BLEND);
		RenderDirectionalLights(camViewMat, camProjMat, modelsAndMats);
		RenderSpotLights(spotLights, modelsAndMats);
		glDisable(GL_BLEND);

		RenderSkybox(camViewMat, camProjMat);
	}


	void DeferredRenderer::RenderGeometry(const Matrix4& camViewMat,
	                                      const Matrix4& camProjMat,
	                                      const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& modelsAndMats)
	{
		m_GBuffer.Clear();
		m_GPassObjectShader.SetUniform("u_ViewProjectionMatrix", camViewMat * camProjMat);

		m_GBuffer.BindForWriting();
		m_GPassObjectShader.Use();

		for (const auto& [modelRes, matrices] : modelsAndMats)
		{
			modelRes->DrawShaded(m_GPassObjectShader, matrices.first, matrices.second, 0);
		}

		m_GPassObjectShader.Unuse();
		m_GBuffer.UnbindFromWriting();
	}


	void DeferredRenderer::RenderAmbientLight()
	{
		m_AmbientShader.SetUniform("u_AmbientLight", AmbientLight::Instance().Intensity());

		static_cast<void>(m_GBuffer.BindForReading(m_AmbientShader, GeometryBuffer::TextureType::Ambient, 0));
		m_AmbientShader.Use();

		glDisable(GL_DEPTH_TEST);
		m_ScreenTexture.Draw();
		glEnable(GL_DEPTH_TEST);

		m_AmbientShader.Unuse();
		m_GBuffer.UnbindFromReading(GeometryBuffer::TextureType::Ambient);
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
		
		auto& lightShader = dirLight->CastsShadow() ? m_ShadowedDirLightShader : m_UnshadowedDirLightShader;

		auto texCount{0};

		texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::TextureType::Position, texCount);
		texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::Normal, texCount);
		texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::TextureType::Diffuse, texCount);
		texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::TextureType::Specular, texCount);
		texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::TextureType::Shine, texCount);

		lightShader.SetUniform("u_DirLight.direction", dirLight->Direction());
		lightShader.SetUniform("u_DirLight.diffuseColor", dirLight->Diffuse());
		lightShader.SetUniform("u_DirLight.specularColor", dirLight->Specular());
		lightShader.SetUniform("u_CameraPosition", Camera::Active()->entity.Transform->Position());

		if (dirLight->CastsShadow())
		{
			static std::vector<Matrix4> dirLightMatrices;
			static std::vector<float> cascadeFarBounds;

			dirLightMatrices.clear();
			cascadeFarBounds.clear();

			const auto cameraInverseMatrix{camViewMat.Inverse()};
			const auto lightViewMatrix{Matrix4::LookAt(dirLight->Range() * -dirLight->Direction(), Vector3{}, Vector3::Up())};
			const auto cascadeCount{Settings::CameraDirectionalShadowCascadeCount()};

			m_ShadowShader.Use();

			for (std::size_t i = 0; i < cascadeCount; ++i)
			{
				const auto lightWorldToClip{m_DirShadowMap.WorldToClipMatrix(i, cameraInverseMatrix, lightViewMatrix)};
				dirLightMatrices.push_back(lightWorldToClip);

				m_ShadowShader.SetUniform("u_LightWorldToClipMatrix", lightWorldToClip);

				m_DirShadowMap.BindForWriting(i);
				m_DirShadowMap.Clear();

				for (const auto& [modelRes, matrices] : modelsAndMats)
				{
					modelRes->DrawDepth(matrices.first);
				}
			}

			m_DirShadowMap.UnbindFromWriting();
			m_ShadowShader.Unuse();

			for (std::size_t i = 0; i < cascadeCount; ++i)
			{
				const auto viewSpaceBound{m_DirShadowMap.CascadeBoundsViewSpace(i)[1]};
				const Vector4 viewSpaceBoundVector{0, 0, viewSpaceBound, 1};
				const auto clipSpaceBoundVector{viewSpaceBoundVector * camProjMat};
				const auto clipSpaceBound{clipSpaceBoundVector[2]};
				cascadeFarBounds.push_back(clipSpaceBound);
			}

			lightShader.SetUniform("u_CascadeCount", static_cast<unsigned>(cascadeCount));
			lightShader.SetUniform("u_LightClipMatrices", dirLightMatrices);
			lightShader.SetUniform("u_CascadeFarBounds", cascadeFarBounds);
			static_cast<void>(m_DirShadowMap.BindForReading(lightShader, texCount));
		}

		lightShader.Use();

		glDisable(GL_DEPTH_TEST);
		m_ScreenTexture.Draw();
		glEnable(GL_DEPTH_TEST);

		lightShader.Unuse();

		if (dirLight->CastsShadow())
		{
			m_DirShadowMap.UnbindFromReading();
		}

		m_GBuffer.UnbindFromReading();
	}


	void DeferredRenderer::RenderSpotLights(const std::vector<const SpotLight*>& spotLights,
	                                        const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& modelsAndMats)
	{
		if (spotLights.empty())
		{
			return;
		}

		auto texCount{0};
		texCount = m_GBuffer.BindForReading(m_SpotLightShader, GeometryBuffer::TextureType::Position, texCount);
		texCount = m_GBuffer.BindForReading(m_SpotLightShader, GeometryBuffer::TextureType::Normal, texCount);
		texCount = m_GBuffer.BindForReading(m_SpotLightShader, GeometryBuffer::TextureType::Diffuse, texCount);
		texCount = m_GBuffer.BindForReading(m_SpotLightShader, GeometryBuffer::TextureType::Specular, texCount);
		texCount = m_GBuffer.BindForReading(m_SpotLightShader, GeometryBuffer::TextureType::Shine, texCount);
		static_cast<void>(m_SpotShadowMap.BindForReading(m_SpotLightShader, texCount));

		m_SpotLightShader.SetUniform("u_CameraPosition", Camera::Active()->entity.Transform->Position());

		for (const auto& spotLight : spotLights)
		{
			const auto lightWorldToClipMat
			{
				Matrix4::LookAt(spotLight->entity.Transform->Position(), spotLight->entity.Transform->Position() + spotLight->entity.Transform->Forward(), Vector3::Up()) *
				Matrix4::Perspective(math::ToRadians(spotLight->OuterAngle() * 2), 1.f, 0.1f, spotLight->Range())
			};

			m_ShadowShader.SetUniform("u_LightWorldToClipMatrix", lightWorldToClipMat);

			m_SpotShadowMap.BindForWriting();
			m_SpotShadowMap.Clear();

			m_ShadowShader.Use();

			for (const auto& [modelRes, matrices] : modelsAndMats)
			{
				modelRes->DrawDepth(matrices.first);
			}

			m_SpotShadowMap.UnbindFromWriting();

			m_SpotLightShader.SetUniform("u_SpotLight.position", spotLight->entity.Transform->Position());
			m_SpotLightShader.SetUniform("u_SpotLight.direction", spotLight->entity.Transform->Forward());
			m_SpotLightShader.SetUniform("u_SpotLight.diffuseColor", spotLight->Diffuse());
			m_SpotLightShader.SetUniform("u_SpotLight.specularColor", spotLight->Specular());
			m_SpotLightShader.SetUniform("u_SpotLight.constant", spotLight->Constant());
			m_SpotLightShader.SetUniform("u_SpotLight.linear", spotLight->Linear());
			m_SpotLightShader.SetUniform("u_SpotLight.quadratic", spotLight->Quadratic());
			m_SpotLightShader.SetUniform("u_SpotLight.range", spotLight->Range());
			m_SpotLightShader.SetUniform("u_SpotLight.innerAngleCosine", math::Cos(math::ToRadians(spotLight->InnerAngle())));
			m_SpotLightShader.SetUniform("u_SpotLight.outerAngleCosine", math::Cos(math::ToRadians(spotLight->OuterAngle())));
			m_SpotLightShader.SetUniform("u_LightWorldToClipMatrix", lightWorldToClipMat);
			m_SpotLightShader.Use();

			glDisable(GL_DEPTH_TEST);
			m_ScreenTexture.Draw();
			glEnable(GL_DEPTH_TEST);
		}
	}


	void DeferredRenderer::RenderSkybox(const Matrix4& camViewMat, const Matrix4& camProjMat)
	{
		const auto& window{Window::Get()};
		m_GBuffer.CopyDepthData(0, Vector2{window.Width(), window.Height()});

		if (const auto& skybox{Camera::Active()->Background().skybox}; skybox.has_value())
		{
			m_SkyboxShader.SetUniform("viewMatrix", static_cast<Matrix4>(static_cast<Matrix3>(camViewMat)));
			m_SkyboxShader.SetUniform("projectionMatrix", camProjMat);

			m_SkyboxShader.Use();
			static_cast<SkyboxResource*>(DataManager::Find(skybox->AllFilePaths()))->Draw(m_SkyboxShader);
		}
	}
}
