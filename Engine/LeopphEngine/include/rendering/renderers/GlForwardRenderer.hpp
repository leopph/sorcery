#pragma once

#include "GlRenderer.hpp"


namespace leopph::internal
{
	class GlForwardRenderer final : public GlRenderer
	{
		struct MappedBuffer
		{
			GLuint name;
			u64 size;
			u8* mapping;
		};


		struct UboGenericData;
		struct UboDirData;
		struct UboSpotData;
		struct UboPointData;


		public:
			auto Render() -> void override;

		private:
			void CreateUbo(u64 index, u64 size);
			void DeleteUbo(u64 index) const;


		public:
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

			std::array<MappedBuffer,3> m_PerFrameUbos{};
			u64 m_PerFrameUboInd{0};
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
