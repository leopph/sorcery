#include "GlMeshGroup.hpp"

#include "../../data/DataManager.hpp"
#include "../../math/Matrix.hpp"

#include <glad/glad.h>

#include <algorithm>
#include <utility>


namespace leopph::internal
{
	GlMeshGroup::GlMeshGroup(std::shared_ptr<const MeshDataGroup> meshDataGroup) :
		m_MeshData{std::move(meshDataGroup)}
	{
		glCreateBuffers(1, &m_InstBuf);
		glNamedBufferData(m_InstBuf, 2 * sizeof(Matrix4), nullptr, GL_STATIC_DRAW);

		std::ranges::for_each(m_MeshData->Data(), [&](const auto& meshData)
		{
			m_Meshes.emplace_back(meshData, m_InstBuf);
		});
	}

	GlMeshGroup::~GlMeshGroup() noexcept
	{
		glDeleteBuffers(1, &m_InstBuf);
	}

	auto GlMeshGroup::DrawShaded(ShaderProgram& shader, const std::size_t nextFreeTextureUnit) const -> void
	{
		for (const auto& mesh : m_Meshes)
		{
			mesh.DrawShaded(shader, nextFreeTextureUnit, m_InstCount);
		}
	}

	auto GlMeshGroup::DrawDepth() const -> void
	{
		for (const auto& mesh : m_Meshes)
		{
			mesh.DrawDepth(m_InstCount);
		}
	}

	auto GlMeshGroup::SetInstanceData(const std::vector<std::pair<Matrix4, Matrix4>>& instMats) const -> void
	{
		m_InstCount = instMats.size();

		if (instMats.size() > m_InstBufSz)
		{
			m_InstBufSz *= 2;
			glNamedBufferData(m_InstBuf, m_InstBufSz * sizeof(std::remove_reference_t<decltype(instMats)>::value_type), instMats.data(), GL_DYNAMIC_DRAW);
		}
		else if (instMats.size() * 2 < m_InstBufSz)
		{
			m_InstBufSz = std::max(m_InstBufSz / 2, 1ull);
			glNamedBufferData(m_InstBuf, m_InstBufSz * sizeof(std::remove_reference_t<decltype(instMats)>::value_type), instMats.data(), GL_DYNAMIC_DRAW);
		}
		else
		{
			glNamedBufferSubData(m_InstBuf, 0, instMats.size() * sizeof(std::remove_reference_t<decltype(instMats)>::value_type), instMats.data());
		}
	}

	auto GlMeshGroup::MeshData() const -> const MeshDataGroup&
	{
		return *m_MeshData;
	}
}
