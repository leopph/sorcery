#include "ForwardRenderer.hpp"

#include "../../components/Camera.hpp"
#include "../../components/lighting/AmbientLight.hpp"
#include "../../components/lighting/DirLight.hpp"
#include "../../components/lighting/Light.hpp"
#include "../../components/lighting/PointLight.hpp"
#include "../../config/Settings.hpp"
#include "../../data/DataManager.hpp"
#include "../../math/LeopphMath.hpp"
#include "../../math/Matrix.hpp"
#include "../../math/Vector.hpp"
#include "../../util/logger.h"

#include <glad/glad.h>

#include <cstddef>
#include <string>
#include <utility>


namespace leopph::internal
{
	ForwardRenderer::ForwardRenderer() :
		m_ObjectShader
		{
			{
				{ShaderFamily::ObjectVertSrc, ShaderType::Vertex},
				{ShaderFamily::ObjectFragSrc, ShaderType::Fragment}
			}
		},
		m_ShadowShader
		{
			{
				{ShaderFamily::ShadowMapVertSrc, ShaderType::Vertex}
			}
		},

		m_SkyboxShader
		{
			{
				{ShaderFamily::SkyboxVertSrc, ShaderType::Vertex},
				{ShaderFamily::SkyboxFragSrc, ShaderType::Fragment}
			}
		}
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		Logger::Instance().Warning("The forward rendering pipeline is currently not feature complete. It is recommended to use the deferred pipeline.");
	}

	auto ForwardRenderer::Render() -> void
	{
		/* We don't render if there is no camera to use */
		if (Camera::Active() == nullptr)
		{
			return;
		}

		const auto& renderables{CollectRenderables()};

		const auto camViewMat{Camera::Active()->ViewMatrix()};
		const auto camProjMat{Camera::Active()->ProjectionMatrix()};

		const auto& dirLight{DataManager::Instance().DirectionalLight()};
		const auto& spotLights{CollectSpotLights()};
		const auto& pointLights{CollectPointLights()};

		RenderShadedObjects(camViewMat, camProjMat, renderables, dirLight, spotLights, pointLights);
		RenderSkybox(camViewMat, camProjMat);
	}

	auto ForwardRenderer::RenderShadedObjects(const Matrix4& camViewMat, const Matrix4& camProjMat, const std::vector<RenderableData>& renderables, const DirectionalLight* dirLight, const std::vector<const SpotLight*>& spotLights, const std::vector<const PointLight*>& pointLights) -> void
	{
		m_ObjectShader.Clear();
		m_ObjectShader["EXISTS_DIRLIGHT"] = std::to_string(dirLight != nullptr);
		m_ObjectShader["DIRLIGHT_SHADOW"] = std::to_string(dirLight != nullptr && dirLight->CastsShadow());
		m_ObjectShader["EXISTS_SPOTLIGHT"] = std::to_string(!spotLights.empty());
		m_ObjectShader["EXISTS_POINTLIGHT"] = std::to_string(!pointLights.empty());
		auto& objectShader{m_ObjectShader.GetPermutation()};
		auto& nonInstShadowShader{m_ShadowShader.GetPermutation()};

		auto texCount{1};

		objectShader.SetUniform("u_ViewProjMat", camViewMat * camProjMat);
		objectShader.SetUniform("u_CamPos", Camera::Active()->Entity()->Transform()->Position());

		/* Set up ambient light data */
		objectShader.SetUniform("u_AmbientLight", AmbientLight::Instance().Intensity());

		/* Set up DirLight data */
		if (dirLight != nullptr)
		{
			objectShader.SetUniform("u_DirLight.direction", dirLight->Direction());
			objectShader.SetUniform("u_DirLight.diffuseColor", dirLight->Diffuse());
			objectShader.SetUniform("u_DirLight.specularColor", dirLight->Specular());

			if (dirLight->CastsShadow())
			{
				static std::vector<Matrix4> dirLightMatrices;
				dirLightMatrices.clear();

				const auto cameraInverseMatrix{camViewMat.Inverse()};
				const auto lightViewMatrix{Matrix4::LookAt(Vector3{0}, dirLight->Direction(), Vector3::Up())};

				const auto cascadeBounds{m_DirLightShadowMap.CalculateCascadeBounds(*Camera::Active())};
				const auto numCascades{cascadeBounds.size()};

				for (std::size_t i = 0; i < numCascades; ++i)
				{
					const auto lightWorldToClip{m_DirLightShadowMap.CascadeMatrix(cascadeBounds[i], cameraInverseMatrix, lightViewMatrix, dirLight->ShadowExtension())};
					dirLightMatrices.push_back(lightWorldToClip);

					nonInstShadowShader.SetUniform("u_ViewProjMat", lightWorldToClip);

					m_DirLightShadowMap.BindForWritingAndClear(i);

					nonInstShadowShader.Use();
					for (const auto& [renderable, instances, castsShadow] : renderables)
					{
						if (castsShadow)
						{
							renderable->SetInstanceData(instances);
							renderable->DrawWithoutMaterial();
						}
					}
				}

				objectShader.SetUniform("u_DirLightCascadeCount", static_cast<unsigned>(numCascades));
				objectShader.SetUniform("u_DirLightClipMatrices", dirLightMatrices);
				objectShader.SetUniform("u_DirLightCascadeFarBounds", CascadeFarBoundsClip(camProjMat, cascadeBounds));
				static_cast<void>(m_DirLightShadowMap.BindForReading(objectShader, "u_DirLightShadowMaps", texCount));
			}
		}

		/* Set up PointLight data */
		objectShader.SetUniform("u_PointLightCount", static_cast<int>(pointLights.size()));
		for (std::size_t i = 0; i < pointLights.size(); i++)
		{
			const auto& pointLight = pointLights[i];

			objectShader.SetUniform("u_PointLights[" + std::to_string(i) + "].position", pointLight->Entity()->Transform()->Position());
			objectShader.SetUniform("u_PointLights[" + std::to_string(i) + "].diffuseColor", pointLight->Diffuse());
			objectShader.SetUniform("u_PointLights[" + std::to_string(i) + "].specularColor", pointLight->Specular());
			objectShader.SetUniform("u_PointLights[" + std::to_string(i) + "].constant", pointLight->Constant());
			objectShader.SetUniform("u_PointLights[" + std::to_string(i) + "].linear", pointLight->Linear());
			objectShader.SetUniform("u_PointLights[" + std::to_string(i) + "].quadratic", pointLight->Quadratic());
		}

		/* Set up SpotLight data */
		objectShader.SetUniform("u_SpotLightCount", static_cast<int>(spotLights.size()));
		for (std::size_t i = 0; i < spotLights.size(); i++)
		{
			const auto& spotLight{spotLights[i]};

			objectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].position", spotLight->Entity()->Transform()->Position());
			objectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].direction", spotLight->Entity()->Transform()->Forward());
			objectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].diffuseColor", spotLight->Diffuse());
			objectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].specularColor", spotLight->Specular());
			objectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].constant", spotLight->Constant());
			objectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].linear", spotLight->Linear());
			objectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].quadratic", spotLight->Quadratic());
			objectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].innerAngleCosine", math::Cos(math::ToRadians(spotLight->InnerAngle())));
			objectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].outerAngleCosine", math::Cos(math::ToRadians(spotLight->OuterAngle())));
		}

		objectShader.Use();
		for (const auto& [renderable, instances, castsShadow] : renderables)
		{
			renderable->SetInstanceData(instances);
			renderable->DrawWithMaterial(objectShader, 0);
		}
	}

	auto ForwardRenderer::RenderSkybox(const Matrix4& camViewMat, const Matrix4& camProjMat) -> void
	{
		if (const auto& background{Camera::Active()->Background()}; std::holds_alternative<Skybox>(background))
		{
			auto& skyboxShader{m_SkyboxShader.GetPermutation()};
			skyboxShader.SetUniform("u_ViewProjMat", static_cast<Matrix4>(static_cast<Matrix3>(camViewMat)) * camProjMat);
			skyboxShader.Use();

			DataManager::Instance().CreateOrGetSkyboxImpl(std::get<Skybox>(background).AllFilePaths())->Draw(skyboxShader);
		}
	}
}
