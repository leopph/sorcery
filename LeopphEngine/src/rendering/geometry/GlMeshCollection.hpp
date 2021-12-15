#pragma once

#include "GlMesh.hpp"
#include "MeshDataCollection.hpp"
#include "../shaders/ShaderProgram.hpp"

#include <cstddef>
#include <memory>
#include <vector>


namespace leopph::impl
{
	class RenderComponent;


	class GlMeshCollection final
	{
		public:
			explicit GlMeshCollection(const MeshDataCollection& modelData);

			GlMeshCollection(const GlMeshCollection& other);
			GlMeshCollection& operator=(const GlMeshCollection& other);

			GlMeshCollection(GlMeshCollection&& other) noexcept;
			GlMeshCollection& operator=(GlMeshCollection&& other) noexcept;

			~GlMeshCollection() noexcept;

			void DrawShaded(ShaderProgram& shader, std::size_t nextFreeTextureUnit) const;
			void DrawDepth() const;

			void AddInstance(const RenderComponent* component) const;
			void RemoveInstance(const RenderComponent* component) const;

			// Matrices must be up-to-date when calling this!
			void UpdateInstanceGeometry() const;

			[[nodiscard]]
			const MeshDataCollection& MeshDataCollection() const;

		private:
			void Deinit() const;

			struct SharedData
			{
				std::vector<GlMesh> Meshes;
				std::vector<const RenderComponent*> RenderInstances;
				unsigned InstanceBuffer{0u};
				std::size_t InstanceBufferSize{1ull};
				std::size_t HandleCount{1ull};
				impl::MeshDataCollection MeshDataCollection;
			};


			std::shared_ptr<SharedData> m_SharedData;
	};
}
