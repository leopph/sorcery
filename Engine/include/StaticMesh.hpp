#pragma once

#include "Material.hpp"
#include "Matrix.hpp"
#include "Types.hpp"
#include "Vertex.hpp"

#include <memory>
#include <span>
#include <vector>


namespace leopph
{
	class StaticMesh
	{
		public:
			struct SubMesh
			{
				u32 first;
				u32 count;
			};


			struct InstanceData
			{
				Matrix4 modelTransp;
				Matrix4 normalTransp;
			};


			[[nodiscard]] u32 get_vao() const;

			[[nodiscard]] InstanceData* get_instance_buffer(std::size_t index) const;
			[[nodiscard]] u32 get_instance_buffer_count() const;
			[[nodiscard]] u32 get_instance_buffer_size() const;
			void resize_instance_buffers(u32 numElements);


			[[nodiscard]] std::span<SubMesh const> get_sub_meshes() const;

			[[nodiscard]] std::span<std::shared_ptr<Material const> const> get_materials() const;
			void set_materials(std::vector<std::shared_ptr<Material const>> materials);

			void set_vertices(std::span<Vertex const> vertices);
			void set_indices(std::span<u32 const> indices);

			StaticMesh();

			StaticMesh(StaticMesh const&) = delete;
			void operator=(StaticMesh const&) = delete;

			StaticMesh(StaticMesh&&) = delete;
			void operator=(StaticMesh&&) = delete;

			~StaticMesh();

		private:
			struct InstanceBuffer
			{ 
				u32 buffer;
				InstanceData* mapping;
			};

			static constexpr u32 NUM_INSTANCE_BUFFERS = 3;

			std::vector<SubMesh> mSubMeshes;
			std::vector<std::shared_ptr<Material const>> mMaterials;
			std::array<InstanceBuffer, NUM_INSTANCE_BUFFERS> mInstanceBufs;
			u32 mInstanceBufSize;
			u32 mVao;
			u32 mVbo;
			u32 mIbo;
	};
}
