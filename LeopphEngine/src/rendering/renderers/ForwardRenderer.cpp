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

#include <glad/gl.h>

#include <cstddef>
#include <string>
#include <utility>



namespace leopph::impl
{
	ForwardRenderer::ForwardRenderer() :
		m_ObjectShader{
			{
				{ShaderFamily::ObjectVertSrc, ShaderType::Vertex},
				{ShaderFamily::ObjectFragSrc, ShaderType::Fragment}
			}},
	m_SkyboxShader{
		{
			{ShaderFamily::SkyboxVertSrc, ShaderType::Vertex},
			{ShaderFamily::SkyboxFragSrc, ShaderType::Fragment}
		}},
	m_ShadowShader{
		{
			{ShaderFamily::ShadowMapVertSrc, ShaderType::Vertex}
		}}
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		Logger::Instance().Warning("The forward rendering pipeline is currently not feature complete. It is recommended to use the deferred pipeline.");
	}


	void ForwardRenderer::Render()
	{
		/* We don't render if there is no camera to use */
		if (Camera::Active == nullptr)
		{
			return;
		}

		UpdateMatrices();

		const auto camViewMat{Camera::Active->ViewMatrix()};
		const auto camProjMat{Camera::Active->ProjectionMatrix()};

		const auto& pointLights{CollectPointLights()};
		const auto& spotLights{CollectSpotLights()};

		RenderShadedObjects(camViewMat, camProjMat, pointLights, spotLights);
		RenderSkybox(camViewMat, camProjMat);
	}


	void ForwardRenderer::RenderShadedObjects(const Matrix4& camViewMat, 
											  const Matrix4& camProjMat,
											  const std::vector<const PointLight*>& pointLights,
											  const std::vector<const SpotLight*>& spotLights)
	{
		static auto shadowShaderFlagInfo{m_ShadowShader.GetFlagInfo()};
		auto& shadowShader{m_ShadowShader.GetPermutation(shadowShaderFlagInfo)};

		static auto objectFlagInfo{m_ObjectShader.GetFlagInfo()};
		objectFlagInfo.Clear();

		const auto dirLight{DataManager::DirectionalLight()};

		objectFlagInfo["EXISTS_DIRLIGHT"] = dirLight != nullptr;
		objectFlagInfo["DIRLIGHT_SHADOW"] = dirLight != nullptr && dirLight->CastsShadow();
		objectFlagInfo["EXISTS_SPOTLIGHT"] = spotLights.size() != 0ull;
		objectFlagInfo["EXISTS_POINTLIGHT"] = pointLights.size() != 0ull;

		auto& objectShader{m_ObjectShader.GetPermutation(objectFlagInfo)};

		auto texCount{1};

		objectShader.SetUniform("u_ViewProjMat", camViewMat * camProjMat);
		objectShader.SetUniform("u_CamPos", Camera::Active->Entity.Transform->Position());

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
				static std::vector<float> cascadeFarBounds;

				dirLightMatrices.clear();
				cascadeFarBounds.clear();

				const auto cameraInverseMatrix{camViewMat.Inverse()};
				const auto lightViewMatrix{Matrix4::LookAt(dirLight->Range() * -dirLight->Direction(), Vector3{}, Vector3::Up())};
				const auto cascadeCount{Settings::DirectionalShadowCascadeCount()};

				shadowShader.Use();

				for (std::size_t i = 0; i < cascadeCount; ++i)
				{
					const auto lightWorldToClip{m_DirLightShadowMap.WorldToClipMatrix(i, cameraInverseMatrix, lightViewMatrix)};
					dirLightMatrices.push_back(lightWorldToClip);

					shadowShader.SetUniform("u_LightWorldToClipMatrix", lightWorldToClip);

					m_DirLightShadowMap.BindForWriting(i);
					m_DirLightShadowMap.Clear();

					for (const auto& renderable : DataManager::Renderables())
					{
						if (renderable->CastsShadow())
						{
							renderable->DrawDepth();
						}
					}
				}

				m_DirLightShadowMap.UnbindFromWriting();
				shadowShader.Unuse();

				for (std::size_t i = 0; i < cascadeCount; ++i)
				{
					const auto viewSpaceBound{m_DirLightShadowMap.CascadeBoundsViewSpace(i)[1]};
					const Vector4 viewSpaceBoundVector{0, 0, viewSpaceBound, 1};
					const auto clipSpaceBoundVector{viewSpaceBoundVector * camProjMat};
					const auto clipSpaceBound{clipSpaceBoundVector[2]};
					cascadeFarBounds.push_back(clipSpaceBound);
				}

				objectShader.SetUniform("u_DirLightCascadeCount", static_cast<unsigned>(cascadeCount));
				objectShader.SetUniform("u_DirLightClipMatrices", dirLightMatrices);
				objectShader.SetUniform("u_DirLightCascadeFarBounds", cascadeFarBounds);
				texCount = m_DirLightShadowMap.BindForReading(objectShader, texCount);
			}
		}

		/* Set up PointLight data */
		objectShader.SetUniform("u_PointLightCount", static_cast<int>(pointLights.size()));
		for (std::size_t i = 0; i < pointLights.size(); i++)
		{
			const auto& pointLight = pointLights[i];

			objectShader.SetUniform("u_PointLights[" + std::to_string(i) + "].position", pointLight->Entity.Transform->Position());
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

			objectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].position", spotLight->Entity.Transform->Position());
			objectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].direction", spotLight->Entity.Transform->Forward());
			objectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].diffuseColor", spotLight->Diffuse());
			objectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].specularColor", spotLight->Specular());
			objectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].constant", spotLight->Constant());
			objectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].linear", spotLight->Linear());
			objectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].quadratic", spotLight->Quadratic());
			objectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].innerAngleCosine", math::Cos(math::ToRadians(spotLight->InnerAngle())));
			objectShader.SetUniform("u_SpotLights[" + std::to_string(i) + "].outerAngleCosine", math::Cos(math::ToRadians(spotLight->OuterAngle())));
		}

		objectShader.Use();

		/* Draw the shaded objects */
		for (const auto& renderable : DataManager::Renderables())
		{
			renderable->DrawShaded(objectShader, texCount);
		}
	}


	void ForwardRenderer::RenderSkybox(const Matrix4& camViewMat, const Matrix4& camProjMat)
	{
		static auto skyboxFlagInfo{m_SkyboxShader.GetFlagInfo()};
		auto& skyboxShader{m_SkyboxShader.GetPermutation(skyboxFlagInfo)};

		if (const auto& skybox{Camera::Active->Background().skybox}; skybox.has_value())
		{
			skyboxShader.Use();
			skyboxShader.SetUniform("viewMatrix", static_cast<Matrix4>(static_cast<Matrix3>(camViewMat)));
			skyboxShader.SetUniform("projectionMatrix", camProjMat);
			DataManager::Skyboxes().find(skybox->AllFilePaths())->first.Draw(skyboxShader);
		}
	}
}
