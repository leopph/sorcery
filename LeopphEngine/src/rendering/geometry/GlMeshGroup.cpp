#include "GlMeshGroup.hpp"

#include "../../data/DataManager.hpp"

#include <algorithm>
#include <utility>


namespace leopph::internal
{
	auto GlMeshGroup::CreateOrGet(std::shared_ptr<internal::MeshGroup const>&& meshGroup) -> std::shared_ptr<GlMeshGroup>
	{
		auto& dataManager = DataManager::Instance();
		auto ret = dataManager.FindGlMeshGroup(meshGroup->Id);

		if (!ret)
		{
			ret = std::shared_ptr<GlMeshGroup>{new GlMeshGroup{std::move(meshGroup)}};
			dataManager.RegisterGlMeshGroup(ret);
		}

		return ret;
	}


	GlMeshGroup::GlMeshGroup(std::shared_ptr<internal::MeshGroup const>&& meshGroup) :
		m_MeshGroup{std::move(meshGroup)}
	{
		glCreateBuffers(1, &m_InstanceBuffer);
		glNamedBufferData(m_InstanceBuffer, 2 * sizeof(Matrix4), nullptr, GL_DYNAMIC_DRAW);

		for (auto const& mesh : *m_MeshGroup->Meshes)
		{
			(IsSemiTransparent(mesh) ? (IsTransparent(mesh) ? m_TransparentMeshes : m_SemiTransparentMeshes) : m_OpaqueMeshes).emplace_back(std::make_unique<GlMesh>(&mesh, m_InstanceBuffer));
		}
	}


	auto GlMeshGroup::DrawWithMaterial(ShaderProgram& shader, GLuint const nextFreeTextureUnit, bool const transparent) const -> void
	{
		for (auto const& mesh : transparent ? m_TransparentMeshes : m_OpaqueMeshes)
		{
			mesh->DrawWithMaterial(shader, nextFreeTextureUnit, m_NumInstances);
		}

		for (auto const& mesh : m_SemiTransparentMeshes)
		{
			mesh->DrawWithMaterial(shader, nextFreeTextureUnit, m_NumInstances);
		}
	}


	auto GlMeshGroup::DrawWithoutMaterial(bool const transparent) const -> void
	{
		for (auto const& mesh : transparent ? m_TransparentMeshes : m_OpaqueMeshes)
		{
			mesh->DrawWithoutMaterial(m_NumInstances);
		}

		for (auto const& mesh : m_SemiTransparentMeshes)
		{
			mesh->DrawWithoutMaterial(m_NumInstances);
		}
	}


	auto GlMeshGroup::SetInstanceData(std::vector<std::pair<Matrix4, Matrix4>> const& instMats) -> void
	{
		if (static_cast<int>(instMats.size()) != m_NumInstances)
		{
			m_NumInstances = static_cast<int>(instMats.size());
			glNamedBufferData(m_InstanceBuffer, static_cast<GLsizei>(m_NumInstances * sizeof(std::remove_reference_t<decltype(instMats)>::value_type)), instMats.data(), GL_DYNAMIC_DRAW);
		}
		else
		{
			glNamedBufferSubData(m_InstanceBuffer, 0, static_cast<GLsizei>(m_NumInstances * sizeof(std::remove_reference_t<decltype(instMats)>::value_type)), instMats.data());
		}
	}


	auto GlMeshGroup::MeshGroup() const -> std::shared_ptr<internal::MeshGroup const> const&
	{
		return m_MeshGroup;
	}


	auto GlMeshGroup::SortMeshes() -> void
	{
		// Move the not opaque meshes out.
		std::ranges::for_each(m_OpaqueMeshes, [this](std::unique_ptr<GlMesh>& glMesh)
		{
			if (IsSemiTransparent(*glMesh->Mesh()))
			{
				(IsTransparent(*glMesh->Mesh()) ? m_TransparentMeshes : m_OpaqueMeshes).emplace_back(std::move(glMesh));
			}
		});

		// Erase the moved-from meshes.
		std::erase_if(m_OpaqueMeshes, [](std::unique_ptr<GlMesh> const& glMesh)
		{
			return !glMesh;
		});

		// Move the opaque and fully transparent meshes out.
		std::ranges::for_each(m_SemiTransparentMeshes, [this](std::unique_ptr<GlMesh>& glMesh)
		{
			if (!IsSemiTransparent(*glMesh->Mesh()))
			{
				m_OpaqueMeshes.emplace_back(std::move(glMesh));
			}
			else if (IsTransparent(*glMesh->Mesh()))
			{
				m_TransparentMeshes.emplace_back(std::move(glMesh));
			}
		});

		// Erase the moved-from meshes.
		std::erase_if(m_SemiTransparentMeshes, [](std::unique_ptr<GlMesh> const& glMesh)
		{
			return !glMesh;
		});

		// Move the not transparent meshes out.
		std::ranges::for_each(m_TransparentMeshes, [this](std::unique_ptr<GlMesh>& glMesh)
		{
			if (!IsTransparent(*glMesh->Mesh()))
			{
				(IsSemiTransparent(*glMesh->Mesh()) ? m_SemiTransparentMeshes : m_OpaqueMeshes).emplace_back(std::move(glMesh));
			}
		});

		// Erase the moved-from meshes.
		std::erase_if(m_TransparentMeshes, [](std::unique_ptr<GlMesh> const& glMesh)
		{
			return !glMesh;
		});
	}


	GlMeshGroup::~GlMeshGroup() noexcept
	{
		glDeleteBuffers(1, &m_InstanceBuffer);
	}


	auto GlMeshGroup::IsSemiTransparent(Mesh const& mesh) -> bool
	{
		return mesh.Material()->DiffuseMap && mesh.Material()->DiffuseMap->IsSemiTransparent() ||
			mesh.Material()->SpecularMap && mesh.Material()->SpecularMap->IsSemiTransparent();
	}


	auto GlMeshGroup::IsTransparent(Mesh const& mesh) -> bool
	{
		return mesh.Material()->DiffuseMap && mesh.Material()->DiffuseMap->IsTransparent() ||
			mesh.Material()->SpecularMap && mesh.Material()->SpecularMap->IsTransparent();
	}
}
