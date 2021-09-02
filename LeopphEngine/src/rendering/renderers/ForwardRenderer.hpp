#pragma once

#include "../../math/Matrix.hpp"
#include "../Shader.hpp"
#include "Renderer.hpp"


namespace leopph::impl
{
	class ForwardRenderer : public Renderer
	{
	public:
		ForwardRenderer();
		void Render() override;

	private:
		void RenderDirectionalShadowMap();
		void RenderPointShadowMaps();
		void RenderShadedObjects();
		void RenderSkybox() const;

		Shader m_ObjectShader;
		Shader m_SkyboxShader;
		Shader m_DirectionalShadowMapShader;

		Matrix4 m_CurrentFrameDirectionalTransformMatrix;
	};
}