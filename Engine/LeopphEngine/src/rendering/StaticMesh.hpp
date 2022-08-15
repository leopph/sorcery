#pragma once

#include "AABB.hpp"
#include "Entity.hpp"
#include "PersistentMappedBuffer.hpp"
#include "StaticModelData.hpp"
#include "SubMeshDescriptor.hpp"

#include <memory>
#include <unordered_set>
#include <vector>


namespace leopph
{
	class StaticMesh : std::enable_shared_from_this<StaticMesh>
	{
		public:
			void draw_sub_mesh(std::size_t index) const;
			[[nodiscard]] std::size_t get_sub_mesh_count() const;

			void set_active_instance_buffer(std::size_t index);

			AABB const& get_bounding_box() const;

			void register_entity(Entity const* entity);
			void unregister_entity(Entity const* entity);

			[[nodiscard]] std::unordered_set<Entity const*> get_entities() const;


			explicit StaticMesh(StaticMeshData const& data);

			StaticMesh(StaticMesh const& other) = delete;
			StaticMesh(StaticMesh&& other) = delete;

			StaticMesh& operator=(StaticMesh const& other) = delete;
			StaticMesh& operator=(StaticMesh&& other) = delete;

			~StaticMesh();


		private:
			u32 mVao{};
			u32 mVbo{};
			u32 mIbo{};

			PersistentMappedBuffer mInstanceBuf;
			u64 mNumInstances{1};

			std::vector<SubMeshDescriptor> mSubMeshes;

			AABB mBoundingBox;

			std::unordered_set<Entity const*> mReferringEntities;
	};
}
