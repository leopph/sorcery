#pragma once

#include "AABB.hpp"
#include "Entity.hpp"
#include "PersistentMappedBuffer.hpp"
#include "StaticModelData.hpp"

#include <memory>
#include <span>
#include <unordered_set>


namespace leopph
{
	class StaticMesh : std::enable_shared_from_this<StaticMesh>
	{
		public:
			void draw() const;

			using InstanceDataType = std::pair<Matrix4, Matrix3>;
			void set_instance_data(std::span<InstanceDataType const> data);

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
			u32 mNumIndices;

			u32 mNumInstances{0};
			std::unique_ptr<PersistentMappedBuffer> mInstanceBuf{std::make_unique<PersistentMappedBuffer>(sizeof InstanceDataType)};

			AABB mBoundingBox;

			std::unordered_set<Entity const*> mReferringEntities;
	};
}
