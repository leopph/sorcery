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

		for (auto const& mesh : m_MeshGroup->Meshes)
		{
			(FullyTransparent(mesh.Material()) ? m_FullyTransparentMeshes : m_OpaqueMeshes).emplace_back(std::make_unique<GlMesh>(&mesh, m_InstanceBuffer));
		}
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
		std::ranges::for_each(m_OpaqueMeshes, [this](std::unique_ptr<GlMesh>& glMesh)
		{
			if (FullyTransparent(glMesh->Mesh()->Material()))
			{
				m_FullyTransparentMeshes.emplace_back(std::move(glMesh));
			}
			else if (MaybeTransparent(glMesh->Mesh()->Material()))
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
			if (!MaybeTransparent(glMesh->Mesh()->Material()))
			{
				(FullyTransparent(glMesh->Mesh()->Material()) ? m_FullyTransparentMeshes : m_OpaqueMeshes).emplace_back(std::move(glMesh));
			}
		});
		
		std::erase_if(m_MaybeTransparentMeshes, [](std::unique_ptr<GlMesh> const& glMesh)
		{
			return !glMesh;
		});
		
		std::ranges::for_each(m_FullyTransparentMeshes, [this](std::unique_ptr<GlMesh>& glMesh)
		{
			if (!FullyTransparent(glMesh->Mesh()->Material()))
			{
				(MaybeTransparent(glMesh->Mesh()->Material()) ? m_MaybeTransparentMeshes : m_OpaqueMeshes).emplace_back(std::move(glMesh));
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
		glDeleteBuffers(1, &m_InstanceBuffer);
	}
}
