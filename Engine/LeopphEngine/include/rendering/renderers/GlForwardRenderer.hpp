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
			static auto CreateUbo(MappedBuffer& ubo, u64 size) -> void;
			static auto DeleteUbo(MappedBuffer const& ubo) -> void;


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

			auto static constexpr TRANSFORM_UBO_SIZE = 6 * sizeof(Matrix4);
			std::array<MappedBuffer, 3> m_TransformUbos{};
			std::array<MappedBuffer, 3> m_LightingUbos{};
			u64 m_PerFrameUboInd{0};
	};



	struct GlForwardRenderer::UboGenericData
	{
		Vector3 cameraPosition;
		alignas(16) Vector3 ambientLight;
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
