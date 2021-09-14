#pragma once

#include "Renderer.hpp"
#include "../../math/Matrix.hpp"
#include "../shaders/DirShadowMapShader.hpp"
#include "../shaders/ForwardObjectShader.hpp"
#include "../shaders/SkyboxShader.hpp"



namespace leopph::impl
{
	class ForwardRenderer final : public Renderer
	{
		public:
			ForwardRenderer();
			void Render() override;

		private:
			void RenderDirectionalShadowMap();
			void RenderPointShadowMaps();
			void RenderShadedObjects();
			void RenderSkybox() const;

			ForwardObjectShader m_ObjectShader;
			SkyboxShader m_SkyboxShader;
			DirShadowMapShader m_DirectionalShadowMapShader;

			Matrix4 m_CurrentFrameDirectionalTransformMatrix;
	};
}
