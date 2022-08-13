#pragma once

#include "PersistentMappedBuffer.hpp"
#include "StaticModelData.hpp"
#include "Texture2D.hpp"

#include <memory>
#include <vector>


namespace leopph
{
	class StaticMaterial
	{
		public:
			void bind_and_set_renderstate(u32 index) const;


			StaticMaterial(StaticMaterialData const& data, std::span<std::shared_ptr<Texture2D const> const> textures);

			StaticMaterial(StaticMaterial const& other) = delete;
			StaticMaterial& operator=(StaticMaterial const& other) = delete;

			StaticMaterial(StaticMaterial&& other) = delete;
			StaticMaterial& operator=(StaticMaterial&& other) = delete;

			~StaticMaterial() = default;


		private:
			PersistentMappedBuffer mBuffer;
			std::vector<std::shared_ptr<Texture2D const>> mTextures;
			bool mCullBackFace;
	};


	class StaticMesh
	{
		public:
			void draw() const;


			StaticMesh(std::span<Vertex const> vertices, std::span<u32 const> indices);

			StaticMesh(StaticMesh const& other) = delete;
			StaticMesh& operator=(StaticMesh const& other) = delete;

			StaticMesh(StaticMesh&& other) = delete;
			StaticMesh& operator=(StaticMesh&& other) = delete;

			~StaticMesh();


		private:
			u32 mVao{};
			u32 mVbo{};
			u32 mIbo{};
			std::size_t mNumIndices;
	};


	class StaticModel
	{
		public:
			explicit StaticModel(StaticModelData const& data);

			StaticModel(StaticModel const& other) = delete;
			StaticModel(StaticModel&& other) = delete;

			StaticModel& operator=(StaticModel const& other) = delete;
			StaticModel& operator=(StaticModel&& other) = delete;

			~StaticModel();

		private:
			std::vector<std::unique_ptr<StaticMesh>> mMeshes;
			std::vector<std::unique_ptr<StaticMaterial>> mMaterials;
			std::vector<std::shared_ptr<Texture2D const>> mTextures;
	};
}
