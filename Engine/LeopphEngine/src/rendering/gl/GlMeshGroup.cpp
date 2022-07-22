#include "rendering/gl/GlMeshGroup.hpp"

#include "DataManager.hpp"

#include <algorithm>
#include <utility>


namespace leopph::internal
{
	GlMeshGroup::GlMeshGroup(leopph::MeshGroup meshGroup) :
		m_MeshGroup{std::move(meshGroup)}
	{
		glNamedBufferData(m_InstanceBuffer, 2 * sizeof(Matrix4), nullptr, GL_DYNAMIC_DRAW);
		UpdateMeshData();
	}


	void GlMeshGroup::DrawWithMaterial(ShaderFamily& shader, GLuint const nextFreeTextureUnit, bool const transparent) const
	{
		for (auto const& mesh : transparent ? m_FullyTransparentMeshes : m_OpaqueMeshes)
		{
			mesh->DrawWithMaterial(shader, nextFreeTextureUnit, m_NumInstances);
		}

		for (auto const& mesh : m_MaybeTransparentMeshes)
		{
			mesh->DrawWithMaterial(shader, nextFreeTextureUnit, m_NumInstances);
		}
	}


	void GlMeshGroup::DrawWithoutMaterial(bool const transparent) const
	{
		for (auto const& mesh : transparent ? m_FullyTransparentMeshes : m_OpaqueMeshes)
		{
			mesh->DrawWithoutMaterial(m_NumInstances);
		}

		for (auto const& mesh : m_MaybeTransparentMeshes)
		{
			mesh->DrawWithoutMaterial(m_NumInstances);
		}
	}


	void GlMeshGroup::SetInstanceData(std::span<std::pair<Matrix4, Matrix4> const> const instMats)
	{
		m_NumInstances = static_cast<GLsizei>(instMats.size());

		// If there are more instances than we what we have space for we reallocate the buffer
		if (m_NumInstances > m_InstanceBufferSize)
		{
			do
			{
				m_InstanceBufferSize *= 2;
			}
			while (m_NumInstances > m_InstanceBufferSize);

			glNamedBufferData(m_InstanceBuffer, static_cast<GLsizei>(m_InstanceBufferSize * sizeof(std::remove_reference_t<decltype(instMats)>::value_type)), nullptr, GL_DYNAMIC_DRAW);
		}

		glNamedBufferSubData(m_InstanceBuffer, 0, static_cast<GLsizei>(m_NumInstances * sizeof(std::remove_reference_t<decltype(instMats)>::value_type)), instMats.data());
	}


	leopph::MeshGroup const& GlMeshGroup::MeshGroup() const
	{
		return m_MeshGroup;
	}


	void GlMeshGroup::MeshGroup(leopph::MeshGroup meshGroup)
	{
		m_MeshGroup = std::move(meshGroup);
		UpdateMeshData();
	}


	void GlMeshGroup::UpdateMeshData()
	{
		m_OpaqueMeshes.clear();
		m_MaybeTransparentMeshes.clear();
		m_FullyTransparentMeshes.clear();

		for (auto const& mesh : m_MeshGroup.Meshes())
		{
			(FullyTransparent(mesh.Material()) ? m_FullyTransparentMeshes : m_OpaqueMeshes).emplace_back(std::make_unique<GlMesh>(mesh, m_InstanceBuffer));
		}
	}


	void GlMeshGroup::SortMeshes()
	{
		std::ranges::for_each(m_OpaqueMeshes, [this](std::unique_ptr<GlMesh>& glMesh)
		{
			if (FullyTransparent(glMesh->Material()))
			{
				m_FullyTransparentMeshes.emplace_back(std::move(glMesh));
			}
			else if (MaybeTransparent(glMesh->Material()))
			{
				m_MaybeTransparentMeshes.emplace_back(std::move(glMesh));
			}
		});

		std::erase_if(m_OpaqueMeshes, [](std::unique_ptr<GlMesh> const& glMesh)
		{
			return !glMesh;
		});

		std::ranges::for_each(m_MaybeTransparentMeshes, [this](std::unique_ptr<GlMesh>& glMesh)
		{
			if (!MaybeTransparent(glMesh->Material()))
			{
				(FullyTransparent(glMesh->Material()) ? m_FullyTransparentMeshes : m_OpaqueMeshes).emplace_back(std::move(glMesh));
			}
		});

		std::erase_if(m_MaybeTransparentMeshes, [](std::unique_ptr<GlMesh> const& glMesh)
		{
			return !glMesh;
		});

		std::ranges::for_each(m_FullyTransparentMeshes, [this](std::unique_ptr<GlMesh>& glMesh)
		{
			if (!FullyTransparent(glMesh->Material()))
			{
				(MaybeTransparent(glMesh->Material()) ? m_MaybeTransparentMeshes : m_OpaqueMeshes).emplace_back(std::move(glMesh));
			}
		});

		std::erase_if(m_FullyTransparentMeshes, [](std::unique_ptr<GlMesh> const& glMesh)
		{
			return !glMesh;
		});
	}


	bool GlMeshGroup::FullyTransparent(std::shared_ptr<Material const> const& mat)
	{
		return mat->Opacity < 1.f;
	}


	bool GlMeshGroup::MaybeTransparent(std::shared_ptr<Material const> const& mat)
	{
		return mat->OpacityMap != nullptr;
	}
}
