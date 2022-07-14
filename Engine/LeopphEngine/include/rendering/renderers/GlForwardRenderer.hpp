#pragma once

#include "GlRenderer.hpp"


namespace leopph::internal
{
	class GlForwardRenderer final : public GlRenderer
	{
		struct Buffer
		{
			GLuint name;
			u64 size;
		};


		struct UboGenericData;
		struct UboDirData;
		struct UboSpotData;
		struct UboPointData;


		public:
			auto Render() -> void override;


			/* ############
			 * RULE OF FIVE
			 * ############ */

			GlForwardRenderer();

			GlForwardRenderer(GlForwardRenderer const& other) = delete;
			auto operator=(GlForwardRenderer const& other) -> void = delete;

			GlForwardRenderer(GlForwardRenderer&& other) = delete;
			auto operator=(GlForwardRenderer&& other) -> void = delete;

			~GlForwardRenderer() override;


		private:
			/* ############
			 * DATA MEMBERS
			 * ############ */

			Buffer m_PerFrameUbo{0, 0};
	};



	struct GlForwardRenderer::UboGenericData
	{
		Matrix4 viewProjMat;
		alignas(16) Vector3 ambientLight;
		alignas(16) Vector3 cameraPosition;
	};



	struct GlForwardRenderer::UboDirData
	{
		Vector3 direction;
		alignas(16) Vector3 diffuse;
		alignas(16) Vector3 specular;
	};



	struct GlForwardRenderer::UboSpotData
	{
		Vector3 position;
		alignas(16) Vector3 direction;
		alignas(16) Vector3 diffuse;
		alignas(16) Vector3 specular;
		f32 range;
		f32 innerCos;
		f32 outerCos;
	};



	struct GlForwardRenderer::UboPointData
	{
		Vector3 position;
		alignas(16) Vector3 diffuse;
		alignas(16) Vector3 specular;
		f32 range;
	};
}
