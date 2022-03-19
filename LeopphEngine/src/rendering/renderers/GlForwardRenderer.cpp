#include "GlForwardRenderer.hpp"

#include "../../components/Camera.hpp"
#include "../../config/Settings.hpp"
#include "../../data/DataManager.hpp"
#include "../../math/Matrix.hpp"
#include "../../util/Logger.hpp"

#include <glad/gl.h>

#include <string>
#include <utility>


namespace leopph::internal
{
	GlForwardRenderer::GlForwardRenderer() :
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
		glDepthFunc(GL_LEQUAL);

		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		Logger::Instance().Warning("The forward rendering pipeline is currently not feature complete. It is recommended to use the deferred pipeline.");
	}


	auto GlForwardRenderer::Render() -> void
	{
		/* We don't render if there is no camera to use */
		if (Camera::Current() == nullptr)
		{
			return;
		}

		static std::vector<RenderableData> renderables;
		static std::vector<SpotLight const*> spotLights;
		static std::vector<PointLight const*> pointLights;

		renderables.clear();
		CollectRenderables(renderables);
		spotLights.clear();
		CollectSpotLights(spotLights);
		pointLights.clear();
		CollectPointLights(pointLights);

		auto const& dirLight{DataManager::Instance().DirectionalLight()};

		auto const camViewMat{Camera::Current()->ViewMatrix()};
		auto const camProjMat{Camera::Current()->ProjectionMatrix()};

		m_RenderBuffer.Clear();
		m_RenderBuffer.BindForWriting();
		RenderShadedObjects(camViewMat, camProjMat, renderables, dirLight, spotLights, pointLights);
		RenderSkybox(camViewMat, camProjMat);
		m_RenderBuffer.CopyColorToDefaultFramebuffer();
	}


	auto GlForwardRenderer::RenderShadedObjects(Matrix4 const& camViewMat, Matrix4 const& camProjMat, std::vector<RenderableData> const& renderables, DirectionalLight const* dirLight, std::vector<SpotLight const*> const& spotLights, std::vector<PointLight const*> const& pointLights) -> void
	{
		m_ObjectShader.Clear();
		m_ObjectShader["DIRLIGHT"] = std::to_string(dirLight != nullptr);
		//m_ObjectShader["DIRLIGHT_SHADOW"] = std::to_string(dirLight != nullptr && dirLight->CastsShadow());
		m_ObjectShader["DIRLIGHT_SHADOW"] = std::to_string(false);
		m_ObjectShader["NUM_SPOTLIGHTS"] = std::to_string(spotLights.size());
		m_ObjectShader["NUM_SPOTLIGHT_SHADOWS"] = std::to_string(0);
		m_ObjectShader["NUM_POINTLIGHTS"] = std::to_string(pointLights.size());
		m_ObjectShader["NUM_POINTLIGHT_SHADOWS"] = std::to_string(0);

		auto& objectShader{m_ObjectShader.GetPermutation()};

		objectShader.SetUniform("u_ViewProjMat", camViewMat * camProjMat);
		objectShader.SetUniform("u_CamPos", Camera::Current()->Entity()->Transform()->Position());

		SetAmbientData(AmbientLight::Instance(), objectShader);
		SetDirectionalData(dirLight, objectShader);
		SetSpotData(spotLights, objectShader);
		SetPointData(pointLights, objectShader);

		objectShader.Use();
		for (auto const& [renderable, instances, castsShadow] : renderables)
		{
			renderable->SetInstanceData(instances);
			renderable->DrawWithMaterial(objectShader, 0, false);
		}
	}


	auto GlForwardRenderer::RenderSkybox(Matrix4 const& camViewMat, Matrix4 const& camProjMat) -> void
	{
		if (auto const& background{Camera::Current()->Background()}; std::holds_alternative<Skybox>(background))
		{
			auto& skyboxShader{m_SkyboxShader.GetPermutation()};
			skyboxShader.SetUniform("u_ViewProjMat", static_cast<Matrix4>(static_cast<Matrix3>(camViewMat)) * camProjMat);
			skyboxShader.Use();

			DataManager::Instance().CreateOrGetSkyboxImpl(std::get<Skybox>(background).AllPaths())->Draw(skyboxShader);
		}
	}
}
