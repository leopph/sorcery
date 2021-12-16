#include "GlMeshGroup.hpp"

#include "../../data/DataManager.hpp"
#include "../../math/Matrix.hpp"

#include <glad/glad.h>

#include <algorithm>
#include <utility>


namespace leopph::impl
{
	GlMeshGroup::GlMeshGroup(const impl::MeshDataGroup& modelData) :
		m_SharedData{std::make_shared_for_overwrite<SharedData>()}
	{
		m_SharedData->MeshData = modelData;

		glCreateBuffers(1, &m_SharedData->InstBuf);
		glNamedBufferData(m_SharedData->InstBuf, 2 * sizeof(Matrix4), nullptr, GL_STATIC_DRAW);

		std::ranges::for_each(modelData.Data(), [&](const auto& meshData)
		{
			m_SharedData->Meshes.emplace_back(meshData, m_SharedData->InstBuf);
		});
	}

	GlMeshGroup::GlMeshGroup(const GlMeshGroup& other) :
		m_SharedData{other.m_SharedData}
	{
		++m_SharedData->RefCount;
	}

	GlMeshGroup& GlMeshGroup::operator=(const GlMeshGroup& other) noexcept
	{
		if (&other == this)
		{
			return *this;
		}

		Deinit();
		m_SharedData = other.m_SharedData;
		++m_SharedData->RefCount;
		return *this;
	}

	GlMeshGroup::GlMeshGroup(GlMeshGroup&& other) noexcept :
		GlMeshGroup{other}
	{}

	GlMeshGroup& GlMeshGroup::operator=(GlMeshGroup&& other)
	{
		return *this = other;
	}

	GlMeshGroup::~GlMeshGroup() noexcept
	{
		Deinit();
	}

	void GlMeshGroup::DrawShaded(leopph::impl::ShaderProgram& shader, const std::size_t nextFreeTextureUnit) const
	{
		for (const auto& mesh : m_SharedData->Meshes)
		{
			mesh.DrawShaded(shader, nextFreeTextureUnit, m_SharedData->InstCount);
		}
	}

	void GlMeshGroup::DrawDepth() const
	{
		for (const auto& mesh : m_SharedData->Meshes)
		{
			mesh.DrawDepth(m_SharedData->InstCount);
		}
	}

	void GlMeshGroup::SetInstanceData(const std::vector<std::pair<Matrix4, Matrix4>>& instMats) const 
	{
		m_SharedData->InstCount = instMats.size();

		if (instMats.size() > m_SharedData->InstBufSz)
		{
			m_SharedData->InstBufSz *= 2;
			glNamedBufferData(m_SharedData->InstBuf, m_SharedData->InstBufSz * sizeof(std::remove_reference_t<decltype(instMats)>::value_type), instMats.data(), GL_DYNAMIC_DRAW);
		}
		else if (instMats.size() * 2 < m_SharedData->InstBufSz)
		{
			m_SharedData->InstBufSz = std::max(m_SharedData->InstBufSz / 2, 1ull);
			glNamedBufferData(m_SharedData->InstBuf, m_SharedData->InstBufSz * sizeof(std::remove_reference_t<decltype(instMats)>::value_type), instMats.data(), GL_DYNAMIC_DRAW);
		}
		else
		{
			glNamedBufferSubData(m_SharedData->InstBuf, 0, instMats.size() * sizeof(std::remove_reference_t<decltype(instMats)>::value_type), instMats.data());
		}
	}

	const MeshDataGroup& GlMeshGroup::MeshData() const
	{
		return m_SharedData->MeshData;
	}

	void GlMeshGroup::Deinit() const
	{
		--m_SharedData->RefCount;

		if (m_SharedData->RefCount == 0)
		{
			glDeleteBuffers(1, &m_SharedData->InstBuf);
		}
	}

}
