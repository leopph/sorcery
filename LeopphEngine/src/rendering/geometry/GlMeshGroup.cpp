#include "GlMeshGroup.hpp"

#include "../../data/DataManager.hpp"

#include <algorithm>
#include <utility>


namespace leopph::internal
{
	GlMeshGroup::GlMeshGroup(leopph::MeshGroup meshGroup) :
		m_MeshGroup{std::move(meshGroup)}
	{
		DataManager::Instance().RegisterGlMeshGroup(this);

		glNamedBufferData(m_InstanceBuffer, 2 * sizeof(Matrix4), nullptr, GL_DYNAMIC_DRAW);
		UpdateMeshData();
	}


	auto GlMeshGroup::DrawWithMaterial(ShaderProgram& shader, GLuint const nextFreeTextureUnit, bool const transparent) const -> void
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


	auto GlMeshGroup::DrawWithoutMaterial(bool const transparent) const -> void
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


	auto GlMeshGroup::SetInstanceData(std::span<std::pair<Matrix4, Matrix4> const> const instMats) -> void
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


	auto GlMeshGroup::MeshGroup() const -> leopph::MeshGroup const&
	{
		return m_MeshGroup;
	}


	auto GlMeshGroup::MeshGroup(leopph::MeshGroup meshGroup) -> void
	{
		m_MeshGroup = std::move(meshGroup);
		UpdateMeshData();
	}


	auto GlMeshGroup::UpdateMeshData() -> void
	{
		m_OpaqueMeshes.clear();
		m_MaybeTransparentMeshes.clear();
		m_FullyTransparentMeshes.clear();

		for (auto const& mesh : m_MeshGroup.Meshes())
		{
			(FullyTransparent(mesh.Material()) ? m_FullyTransparentMeshes : m_OpaqueMeshes).emplace_back(std::make_unique<GlMesh>(mesh, m_InstanceBuffer));
		}
	}


	auto GlMeshGroup::SortMeshes() -> void
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


	auto GlMeshGroup::FullyTransparent(std::shared_ptr<Material const> const& mat) -> bool
	{
		return mat->Opacity < 1.f;
	}


	auto GlMeshGroup::MaybeTransparent(std::shared_ptr<Material const> const& mat) -> bool
	{
		return mat->OpacityMap != nullptr;
	}


	GlMeshGroup::~GlMeshGroup() noexcept
	{
		DataManager::Instance().UnregisterGlMeshGroup(this);
	}
}
