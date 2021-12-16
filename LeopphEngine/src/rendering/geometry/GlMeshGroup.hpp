#pragma once

#include "GlMesh.hpp"
#include "MeshDataGroup.hpp"
#include "../shaders/ShaderProgram.hpp"

#include <cstddef>
#include <memory>
#include <vector>


namespace leopph::impl
{
	class RenderComponent;


	class GlMeshGroup final
	{
		public:
			explicit GlMeshGroup(const MeshDataGroup& modelData);

			GlMeshGroup(const GlMeshGroup& other);
			GlMeshGroup& operator=(const GlMeshGroup& other);

			GlMeshGroup(GlMeshGroup&& other) noexcept;
			GlMeshGroup& operator=(GlMeshGroup&& other) noexcept;

			~GlMeshGroup() noexcept;

			void DrawShaded(ShaderProgram& shader, std::size_t nextFreeTextureUnit) const;
			void DrawDepth() const;

			void AddInstance(const RenderComponent* component) const;
			void RemoveInstance(const RenderComponent* component) const;

			// Matrices must be up-to-date when calling this!
			void UpdateInstanceGeometry() const;

			[[nodiscard]]
			const MeshDataGroup& MeshDataCollection() const;

		private:
			void Deinit() const;

			struct SharedData
			{
				std::vector<GlMesh> Meshes;
				std::vector<const RenderComponent*> RenderInstances;
				unsigned InstanceBuffer{0u};
				std::size_t InstanceBufferSize{1ull};
				std::size_t HandleCount{1ull};
				impl::MeshDataGroup MeshDataCollection;
			};


			std::shared_ptr<SharedData> m_SharedData;
	};
}
