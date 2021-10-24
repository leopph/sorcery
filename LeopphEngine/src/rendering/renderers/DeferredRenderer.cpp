#include "DeferredRenderer.hpp"

#include "../../components/Camera.hpp"
#include "../../components/lighting/AmbientLight.hpp"
#include "../../config/Settings.hpp"
#include "../../data/DataManager.hpp"
#include "../../math/LeopphMath.hpp"
#include "../../math/Matrix.hpp"
#include "../../windowing/Window.hpp"

#include <glad/glad.h>

#include <array>



namespace leopph::impl
{
	DeferredRenderer::DeferredRenderer() :
		m_TextureShader{
			{
				{ShaderFamily::LightPassVertSrc, ShaderType::Vertex},
				{ShaderFamily::TextureFragSrc, ShaderType::Fragment}
			}
		},
		m_ShadowShader{
			{
				{ShaderFamily::ShadowMapVertSrc, ShaderType::Vertex}
			}
		},
		m_CubeShadowShader{
			{
				{ShaderFamily::CubeShadowMapVertSrc, ShaderType::Vertex},
				{ShaderFamily::CubeShadowMapGeomSrc, ShaderType::Geometry},
				{ShaderFamily::CubeShadowMapFragSrc, ShaderType::Fragment}
			}
		},
		m_GeometryShader{
			{
				{ShaderFamily::GPassObjectVertSrc, ShaderType::Vertex},
				{ShaderFamily::GPassObjectFragSrc, ShaderType::Fragment}
			}
		},
		m_SkyboxShader{
			{
				{ShaderFamily::SkyboxVertSrc, ShaderType::Vertex},
				{ShaderFamily::SkyboxFragSrc, ShaderType::Fragment}
			}
		},
		m_AmbientShader{
			{
				{ShaderFamily::LightPassVertSrc, ShaderType::Vertex},
				{ShaderFamily::AmbLightFragSrc, ShaderType::Fragment}
			}
		},
		m_DirLightShader{
			{
				{ShaderFamily::LightPassVertSrc, ShaderType::Vertex},
				{ShaderFamily::DirLightPassFragSrc, ShaderType::Fragment}
			}
		},
		m_SpotLightShader{
			{
				{ShaderFamily::LightPassVertSrc, ShaderType::Vertex},
				{ShaderFamily::SpotLightPassFragSrc, ShaderType::Fragment}
			}
		},
		m_PointLightShader{
			{
				{ShaderFamily::LightPassVertSrc, ShaderType::Vertex},
				{ShaderFamily::PointLightPassFragSrc, ShaderType::Fragment}
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
		RenderPointLights(pointLights, modelsAndMats);
		glDisable(GL_BLEND);

		RenderSkybox(camViewMat, camProjMat);
	}


	void DeferredRenderer::RenderGeometry(const Matrix4& camViewMat,
	                                      const Matrix4& camProjMat,
	                                      const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& modelsAndMats)
	{
		static auto gPassFlagInfo{m_GeometryShader.GetFlagInfo()};
		auto& shader{m_GeometryShader.GetPermutation(gPassFlagInfo)};

		m_GBuffer.Clear();

		shader.SetUniform("u_ViewProjectionMatrix", camViewMat * camProjMat);

		m_GBuffer.BindForWriting();
		shader.Use();

		for (const auto& [modelRes, matrices] : modelsAndMats)
		{
			modelRes->DrawShaded(shader, matrices.first, matrices.second, 0);
		}

		shader.Unuse();
		m_GBuffer.UnbindFromWriting();
	}


	void DeferredRenderer::RenderAmbientLight()
	{
		static auto ambientFlagInfo{m_AmbientShader.GetFlagInfo()};
		auto& shader{m_AmbientShader.GetPermutation(ambientFlagInfo)};
		shader.SetUniform("u_AmbientLight", AmbientLight::Instance().Intensity());

		static_cast<void>(m_GBuffer.BindForReading(shader, GeometryBuffer::TextureType::Ambient, 0));
		shader.Use();

		glDisable(GL_DEPTH_TEST);
		m_ScreenTexture.Draw();
		glEnable(GL_DEPTH_TEST);

		shader.Unuse();
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

		static auto lightShaderFlagInfo{m_DirLightShader.GetFlagInfo()};
		static auto shadowShaderFlagInfo{m_ShadowShader.GetFlagInfo()};

		lightShaderFlagInfo["CAST_SHADOW"] = dirLight->CastsShadow();

		auto& lightShader{m_DirLightShader.GetPermutation(lightShaderFlagInfo)};
		auto& shadowShader{m_ShadowShader.GetPermutation(shadowShaderFlagInfo)};

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

			shadowShader.Use();

			for (std::size_t i = 0; i < cascadeCount; ++i)
			{
				const auto lightWorldToClip{m_DirShadowMap.WorldToClipMatrix(i, cameraInverseMatrix, lightViewMatrix)};
				dirLightMatrices.push_back(lightWorldToClip);

				shadowShader.SetUniform("u_LightWorldToClipMatrix", lightWorldToClip);

				m_DirShadowMap.BindForWriting(i);
				m_DirShadowMap.Clear();

				for (const auto& [modelRes, matrices] : modelsAndMats)
				{
					modelRes->DrawDepth(matrices.first);
				}
			}

			m_DirShadowMap.UnbindFromWriting();
			shadowShader.Unuse();

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

		static auto lightFlagInfo{m_SpotLightShader.GetFlagInfo()};
		static auto shadowFlagInfo{m_ShadowShader.GetFlagInfo()};

		auto& shadowShader{m_ShadowShader.GetPermutation(shadowFlagInfo)};

		for (const auto& spotLight : spotLights)
		{
			lightFlagInfo.Clear();
			lightFlagInfo["CAST_SHADOW"] = spotLight->CastsShadow();
			auto& lightShader{m_SpotLightShader.GetPermutation(lightFlagInfo)};

			auto texCount{0};
			texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::TextureType::Position, texCount);
			texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::TextureType::Normal, texCount);
			texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::TextureType::Diffuse, texCount);
			texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::TextureType::Specular, texCount);
			texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::TextureType::Shine, texCount);
			static_cast<void>(m_SpotShadowMap.BindForReading(lightShader, texCount));

			lightShader.SetUniform("u_CameraPosition", Camera::Active()->entity.Transform->Position());

			const auto lightWorldToClipMat
			{
				Matrix4::LookAt(spotLight->entity.Transform->Position(), spotLight->entity.Transform->Position() + spotLight->entity.Transform->Forward(), Vector3::Up()) *
				Matrix4::Perspective(math::ToRadians(spotLight->OuterAngle() * 2), 1.f, 0.1f, spotLight->Range())
			};

			shadowShader.SetUniform("u_LightWorldToClipMatrix", lightWorldToClipMat);

			m_SpotShadowMap.BindForWriting();
			m_SpotShadowMap.Clear();

			shadowShader.Use();

			for (const auto& [modelRes, matrices] : modelsAndMats)
			{
				modelRes->DrawDepth(matrices.first);
			}

			m_SpotShadowMap.UnbindFromWriting();

			lightShader.SetUniform("u_SpotLight.position", spotLight->entity.Transform->Position());
			lightShader.SetUniform("u_SpotLight.direction", spotLight->entity.Transform->Forward());
			lightShader.SetUniform("u_SpotLight.diffuseColor", spotLight->Diffuse());
			lightShader.SetUniform("u_SpotLight.specularColor", spotLight->Specular());
			lightShader.SetUniform("u_SpotLight.constant", spotLight->Constant());
			lightShader.SetUniform("u_SpotLight.linear", spotLight->Linear());
			lightShader.SetUniform("u_SpotLight.quadratic", spotLight->Quadratic());
			lightShader.SetUniform("u_SpotLight.range", spotLight->Range());
			lightShader.SetUniform("u_SpotLight.innerAngleCosine", math::Cos(math::ToRadians(spotLight->InnerAngle())));
			lightShader.SetUniform("u_SpotLight.outerAngleCosine", math::Cos(math::ToRadians(spotLight->OuterAngle())));
			lightShader.SetUniform("u_LightWorldToClipMatrix", lightWorldToClipMat);
			lightShader.Use();

			glDisable(GL_DEPTH_TEST);
			m_ScreenTexture.Draw();
			glEnable(GL_DEPTH_TEST);
		}
	}


	void DeferredRenderer::RenderPointLights(const std::vector<const PointLight*>& pointLights,
	                                         const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& modelsAndMats)
	{
		if (pointLights.empty())
		{
			return;
		}

		static auto lightShaderFlagInfo{m_PointLightShader.GetFlagInfo()};
		static auto& shadowShader{m_CubeShadowShader.GetPermutation(m_CubeShadowShader.GetFlagInfo())};

		static std::vector<Matrix4> shadowViewProjMats;

		for (const auto& pointLight : pointLights)
		{
			const auto& lightPos{pointLight->entity.Transform->Position()};

			lightShaderFlagInfo.Clear();
			lightShaderFlagInfo["CAST_SHADOW"] = pointLight->CastsShadow();
			auto& lightShader{m_PointLightShader.GetPermutation(lightShaderFlagInfo)};

			auto texCount{0};
			texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::TextureType::Position, texCount);
			texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::TextureType::Normal, texCount);
			texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::TextureType::Diffuse, texCount);
			texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::TextureType::Specular, texCount);
			texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::TextureType::Shine, texCount);

			lightShader.SetUniform("u_PointLight.position", lightPos);
			lightShader.SetUniform("u_PointLight.diffuseColor", pointLight->Diffuse());
			lightShader.SetUniform("u_PointLight.specularColor", pointLight->Specular());
			lightShader.SetUniform("u_PointLight.constant", pointLight->Constant());
			lightShader.SetUniform("u_PointLight.linear", pointLight->Linear());
			lightShader.SetUniform("u_PointLight.quadratic", pointLight->Quadratic());
			lightShader.SetUniform("u_PointLight.range", pointLight->Range());
			lightShader.SetUniform("u_CamPos", Camera::Active()->entity.Transform->Position());

			if (pointLight->CastsShadow())
			{
				static_cast<void>(m_PointShadowMap.BindForReading(lightShader, texCount));

				const auto shadowProj{Matrix4::Perspective(math::ToRadians(90), 1, 0.01f, pointLight->Range())};

				shadowViewProjMats.clear();

				for (static const std::array directionData
				     {
						std::pair{Vector3::Right(), Vector3::Up()},
						std::pair{Vector3::Left(), Vector3::Up()},
						std::pair{Vector3::Up(), Vector3::Backward()},
						std::pair{Vector3::Down(), Vector3::Forward()},
						std::pair{Vector3::Forward(), Vector3::Up()},
						std::pair{Vector3::Backward(), Vector3::Up()}
				     }; const auto& [target, up] : directionData)
				{
					shadowViewProjMats.emplace_back(Matrix4::LookAt(lightPos, lightPos + target, up) * shadowProj);
				}

				shadowShader.SetUniform("u_ViewProjMats", shadowViewProjMats);
				shadowShader.SetUniform("u_LightPos", lightPos);
				shadowShader.SetUniform("u_FarPlane", pointLight->Range());

				m_PointShadowMap.Clear();

				m_PointShadowMap.BindForWriting();
				shadowShader.Use();

				for (const auto& [modelRes, matrices] : modelsAndMats)
				{
					modelRes->DrawDepth(matrices.first);
				}

				shadowShader.Unuse();
				m_PointShadowMap.UnbindFromWriting();
			}

			lightShader.Use();

			glDisable(GL_DEPTH_TEST);
			m_ScreenTexture.Draw();
			glEnable(GL_DEPTH_TEST);

			lightShader.Unuse();
		}
	}


	void DeferredRenderer::RenderSkybox(const Matrix4& camViewMat, const Matrix4& camProjMat)
	{
		static auto skyboxFlagInfo{m_SkyboxShader.GetFlagInfo()};
		auto& skyboxShader{m_SkyboxShader.GetPermutation(skyboxFlagInfo)};

		const auto& window{Window::Get()};
		m_GBuffer.CopyDepthData(0, Vector2{window.Width(), window.Height()});

		if (const auto& skybox{Camera::Active()->Background().skybox}; skybox.has_value())
		{
			skyboxShader.SetUniform("viewMatrix", static_cast<Matrix4>(static_cast<Matrix3>(camViewMat)));
			skyboxShader.SetUniform("projectionMatrix", camProjMat);

			skyboxShader.Use();
			static_cast<SkyboxResource*>(DataManager::Find(skybox->AllFilePaths()))->Draw(skyboxShader);
		}
	}
}
