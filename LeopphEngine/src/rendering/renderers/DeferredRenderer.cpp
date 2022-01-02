#include "DeferredRenderer.hpp"

#include "../../components/Camera.hpp"
#include "../../components/lighting/AmbientLight.hpp"
#include "../../data/DataManager.hpp"
#include "../../math/LeopphMath.hpp"
#include "../../math/Matrix.hpp"

#include <glad/glad.h>

#include <algorithm>
#include <array>
#include <iterator>


namespace leopph::internal
{
	DeferredRenderer::DeferredRenderer() :
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

		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		glEnable(GL_STENCIL_TEST);
	}


	auto DeferredRenderer::Render() -> void
	{
		/* We don't render if there is no camera to use */
		if (Camera::Active() == nullptr)
		{
			return;
		}

		const auto& renderables{CollectRenderables()};

		const auto camViewMat{Camera::Active()->ViewMatrix()};
		const auto camProjMat{Camera::Active()->ProjectionMatrix()};

		const auto& pointLights{CollectPointLights()};
		const auto& spotLights{CollectSpotLights()};

		glStencilFunc(GL_ALWAYS, STENCIL_REF, STENCIL_AND_MASK);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		RenderGeometry(camViewMat, camProjMat, renderables);

		m_RenderBuffer.Clear();
		m_GBuffer.CopyStencilData(m_RenderBuffer.FramebufferName());

		glStencilFunc(GL_EQUAL, STENCIL_REF, STENCIL_AND_MASK);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		RenderAmbientLight();
		glEnable(GL_BLEND);
		RenderDirectionalLights(camViewMat, camProjMat, renderables);
		RenderSpotLights(spotLights, renderables);
		RenderPointLights(pointLights, renderables);
		glDisable(GL_BLEND);

		glStencilFunc(GL_NOTEQUAL, STENCIL_REF, STENCIL_AND_MASK);
		RenderSkybox(camViewMat, camProjMat);

		m_RenderBuffer.CopyColorToDefaultBuffer();
	}


	auto DeferredRenderer::RenderGeometry(const Matrix4& camViewMat, const Matrix4& camProjMat, const std::vector<RenderableData>& renderables) -> void
	{
		static auto flagInfo{m_GeometryShader.GetFlagInfo()};
		flagInfo.Clear();
		auto& shader{m_GeometryShader.GetPermutation(flagInfo)};

		m_GBuffer.Clear();

		shader.SetUniform("u_ViewProjMat", camViewMat * camProjMat);

		m_GBuffer.BindForWriting();

		shader.Use();
		for (const auto& [renderable, instances, castsShadow] : renderables)
		{
			renderable->SetInstanceData(instances);
			renderable->DrawWithMaterial(shader, 0);
		}

		GeometryBuffer::UnbindFromWriting();
	}


	auto DeferredRenderer::RenderAmbientLight() -> void
	{
		static auto ambientFlagInfo{m_AmbientShader.GetFlagInfo()};
		auto& shader{m_AmbientShader.GetPermutation(ambientFlagInfo)};

		shader.SetUniform("u_AmbientLight", AmbientLight::Instance().Intensity());

		static_cast<void>(m_GBuffer.BindForReading(shader, GeometryBuffer::Texture::Ambient, 0));
		shader.Use();

		m_RenderBuffer.DrawQuad();
	}


	auto DeferredRenderer::RenderDirectionalLights(const Matrix4& camViewMat, const Matrix4& camProjMat, const std::vector<RenderableData>& renderables) -> void
	{
		const auto& dirLight{DataManager::Instance().DirectionalLight()};

		if (dirLight == nullptr)
		{
			return;
		}

		static auto lightFlagInfo{m_DirLightShader.GetFlagInfo()};
		lightFlagInfo.Clear();
		lightFlagInfo["CAST_SHADOW"] = dirLight->CastsShadow();
		auto& lightShader{m_DirLightShader.GetPermutation(lightFlagInfo)};

		static auto shadowFlagInfo{m_ShadowShader.GetFlagInfo()};
		shadowFlagInfo.Clear();
		auto& shadowShader{m_ShadowShader.GetPermutation(shadowFlagInfo)};

		auto texCount{0};

		texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::Texture::Position, texCount);
		texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::Texture::Normal, texCount);
		texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::Texture::Diffuse, texCount);
		texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::Texture::Specular, texCount);
		texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::Texture::Shine, texCount);

		lightShader.SetUniform("u_DirLight.direction", dirLight->Direction());
		lightShader.SetUniform("u_DirLight.diffuseColor", dirLight->Diffuse());
		lightShader.SetUniform("u_DirLight.specularColor", dirLight->Specular());
		lightShader.SetUniform("u_CameraPosition", Camera::Active()->Entity()->Transform()->Position());

		if (dirLight->CastsShadow())
		{
			static std::vector<Matrix4> cascadeMats;
			cascadeMats.clear();

			const auto camInvMat{camViewMat.Inverse()};
			const auto lightViewMat{Matrix4::LookAt(Vector3{0}, dirLight->Direction(), Vector3::Up())};

			const auto cascadeBounds{m_DirShadowMap.CalculateCascadeBounds(*Camera::Active())};
			const auto numCascades{cascadeBounds.size()};

			for (std::size_t i = 0; i < numCascades; ++i)
			{
				const auto cascadeMat{m_DirShadowMap.CascadeMatrix(cascadeBounds[i], camInvMat, lightViewMat, dirLight->ShadowExtension())};
				cascadeMats.push_back(cascadeMat);

				shadowShader.SetUniform("u_WorldToClipMat", cascadeMat);

				m_DirShadowMap.BindForWriting(i);
				m_DirShadowMap.Clear();

				shadowShader.Use();
				for (const auto& [renderable, instances, castsShadow] : renderables)
				{
					if (castsShadow)
					{
						renderable->SetInstanceData(instances);
						renderable->DrawWithoutMaterial();
					}
				}
			}

			CascadedShadowMap::UnbindFromWriting();

			lightShader.SetUniform("u_NumCascades", static_cast<unsigned>(numCascades));
			lightShader.SetUniform("u_CascadeMatrices", cascadeMats);
			lightShader.SetUniform("u_CascadeBounds", CascadeFarBoundsClip(camProjMat, cascadeBounds));
			static_cast<void>(m_DirShadowMap.BindForReading(lightShader, texCount));
		}

		lightShader.Use();
		m_RenderBuffer.DrawQuad();
	}


	auto DeferredRenderer::RenderSpotLights(const std::vector<const SpotLight*>& spotLights, const std::vector<RenderableData>& renderables) -> void
	{
		if (spotLights.empty())
		{
			return;
		}

		static auto shadowFlagInfo{m_ShadowShader.GetFlagInfo()};
		shadowFlagInfo.Clear();
		auto& shadowShader{m_ShadowShader.GetPermutation(shadowFlagInfo)};

		for (const auto& spotLight : spotLights)
		{
			static auto lightFlagInfo{m_DirLightShader.GetFlagInfo()};
			lightFlagInfo.Clear();
			lightFlagInfo["CAST_SHADOW"] = spotLight->CastsShadow();
			auto& lightShader{m_SpotLightShader.GetPermutation(lightFlagInfo)};

			auto texCount{0};

			texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::Texture::Position, texCount);
			texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::Texture::Normal, texCount);
			texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::Texture::Diffuse, texCount);
			texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::Texture::Specular, texCount);
			texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::Texture::Shine, texCount);
			static_cast<void>(m_SpotShadowMap.BindForReading(lightShader, texCount));

			lightShader.SetUniform("u_CamPos", Camera::Active()->Entity()->Transform()->Position());

			const auto lightWorldToClipMat
			{
				Matrix4::LookAt(spotLight->Entity()->Transform()->Position(), spotLight->Entity()->Transform()->Position() + spotLight->Entity()->Transform()->Forward(), Vector3::Up()) *
				Matrix4::Perspective(math::ToRadians(spotLight->OuterAngle() * 2), 1.f, 0.1f, spotLight->Range())
			};

			shadowShader.SetUniform("u_WorldToClipMat", lightWorldToClipMat);

			m_SpotShadowMap.BindForWriting();
			m_SpotShadowMap.Clear();

			shadowShader.Use();
			for (const auto& [renderable, instances, castsShadow] : renderables)
			{
				if (castsShadow)
				{
					renderable->SetInstanceData(instances);
					renderable->DrawWithoutMaterial();
				}
			}

			m_SpotShadowMap.UnbindFromWriting();

			lightShader.SetUniform("u_SpotLight.position", spotLight->Entity()->Transform()->Position());
			lightShader.SetUniform("u_SpotLight.direction", spotLight->Entity()->Transform()->Forward());
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
			m_RenderBuffer.DrawQuad();
		}
	}


	auto DeferredRenderer::RenderPointLights(const std::vector<const PointLight*>& pointLights, const std::vector<RenderableData>& renderables) -> void
	{
		if (pointLights.empty())
		{
			return;
		}

		static auto shadowFlagInfo{m_CubeShadowShader.GetFlagInfo()};
		shadowFlagInfo.Clear();
		auto& shadowShader{m_CubeShadowShader.GetPermutation(shadowFlagInfo)};

		for (const auto& pointLight : pointLights)
		{
			static auto lightFlagInfo{m_PointLightShader.GetFlagInfo()};
			lightFlagInfo.Clear();
			lightFlagInfo["CAST_SHADOW"] = pointLight->CastsShadow();
			auto& lightShader{m_PointLightShader.GetPermutation(lightFlagInfo)};

			auto texCount{0};
			texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::Texture::Position, texCount);
			texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::Texture::Normal, texCount);
			texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::Texture::Diffuse, texCount);
			texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::Texture::Specular, texCount);
			texCount = m_GBuffer.BindForReading(lightShader, GeometryBuffer::Texture::Shine, texCount);

			lightShader.SetUniform("u_PointLight.position", pointLight->Entity()->Transform()->Position());
			lightShader.SetUniform("u_PointLight.diffuseColor", pointLight->Diffuse());
			lightShader.SetUniform("u_PointLight.specularColor", pointLight->Specular());
			lightShader.SetUniform("u_PointLight.constant", pointLight->Constant());
			lightShader.SetUniform("u_PointLight.linear", pointLight->Linear());
			lightShader.SetUniform("u_PointLight.quadratic", pointLight->Quadratic());
			lightShader.SetUniform("u_PointLight.range", pointLight->Range());
			lightShader.SetUniform("u_CamPos", Camera::Active()->Entity()->Transform()->Position());

			if (pointLight->CastsShadow())
			{
				static_cast<void>(m_PointShadowMap.BindForReading(lightShader, texCount));

				const auto shadowProj{Matrix4::Perspective(math::ToRadians(90), 1, 0.01f, pointLight->Range())};

				static std::vector<Matrix4> shadowViewProjMats;
				shadowViewProjMats.clear();

				static constexpr std::array cubeFaceMats
				{
					Matrix4{0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 0, 0, 0, 1}, // +X
					Matrix4{0, 0, -1, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1}, // -X
					Matrix4{1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1}, // +Y
					Matrix4{1, 0, 0, 0, 0, 0, -1, 0, 0, -1, 0, 0, 0, 0, 0, 1}, // -Y
					Matrix4{1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, // +Z
					Matrix4{-1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1} // -Z
				};

				std::ranges::transform(cubeFaceMats, std::back_inserter(shadowViewProjMats), [&](const auto& cubeFaceMat)
				{
					return Matrix4::Translate(-pointLight->Entity()->Transform()->Position()) * cubeFaceMat * shadowProj;
				});

				shadowShader.SetUniform("u_ViewProjMats", shadowViewProjMats);
				shadowShader.SetUniform("u_LightPos", pointLight->Entity()->Transform()->Position());
				shadowShader.SetUniform("u_FarPlane", pointLight->Range());

				m_PointShadowMap.BindForWriting();
				m_PointShadowMap.Clear();

				shadowShader.Use();
				for (const auto& [renderable, instances, castsShadow] : renderables)
				{
					if (castsShadow)
					{
						renderable->SetInstanceData(instances);
						renderable->DrawWithoutMaterial();
					}
				}

				m_PointShadowMap.UnbindFromWriting();
			}

			lightShader.Use();
			m_RenderBuffer.DrawQuad();
		}
	}


	auto DeferredRenderer::RenderSkybox(const Matrix4& camViewMat, const Matrix4& camProjMat) -> void
	{
		if (const auto& background{Camera::Active()->Background()}; std::holds_alternative<Skybox>(background))
		{
			static auto skyboxFlagInfo{m_SkyboxShader.GetFlagInfo()};
			auto& skyboxShader{m_SkyboxShader.GetPermutation(skyboxFlagInfo)};

			skyboxShader.SetUniform("u_ViewProjMat", static_cast<Matrix4>(static_cast<Matrix3>(camViewMat)) * camProjMat);
			m_RenderBuffer.BindAsRenderTarget();
			skyboxShader.Use();
			DataManager::Instance().CreateOrGetSkyboxImpl(std::get<Skybox>(background).AllFilePaths())->Draw(skyboxShader);
			RenderBuffer::UnbindAsRenderTarget();
		}
	}
}
