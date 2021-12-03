#include "InstancedRenderable.hpp"

#include <glad/gl.h>

#include <functional>
#include <type_traits>


namespace leopph::impl
{
	InstancedRenderable::InstancedRenderable(ModelData& modelData) :
		Renderable{modelData},
		m_Meshes{},
		m_InstanceBuffer{},
		m_InstanceBufferSize{1},
		m_InstanceCount{0ull}
	{
		glCreateBuffers(1, &m_InstanceBuffer);
		glNamedBufferData(m_InstanceBuffer, 2 * sizeof(Matrix4), nullptr, GL_STATIC_DRAW);

		std::ranges::for_each(modelData.MeshData, [&](auto& meshData)
		{
			m_Meshes.emplace_back(std::make_unique<InstancedMesh>(meshData, m_InstanceBuffer));
		});
	}


	void InstancedRenderable::DrawShaded(leopph::impl::ShaderProgram& shader, const std::size_t nextFreeTextureUnit) const
	{
		for (const auto& mesh : m_Meshes)
		{
			mesh->DrawShaded(shader, nextFreeTextureUnit, m_InstanceCount);
		}
	}


	void InstancedRenderable::DrawDepth() const
	{
		for (const auto& mesh : m_Meshes)
		{
			mesh->DrawDepth(m_InstanceCount);
		}
	}


	void InstancedRenderable::SetInstanceData(const std::vector<std::pair<Matrix4, Matrix4>>& instanceMatrices) const
	{
		m_InstanceCount = instanceMatrices.size();

		if (instanceMatrices.size() > m_InstanceBufferSize)
		{
			m_InstanceBufferSize *= 2;
			glNamedBufferData(m_InstanceBuffer, m_InstanceBufferSize * sizeof(std::remove_reference_t<decltype(instanceMatrices)>::value_type), instanceMatrices.data(), GL_STATIC_DRAW);
		}
		else if (instanceMatrices.size() * 2 < m_InstanceBufferSize)
		{
			m_InstanceBufferSize = std::max(m_InstanceBufferSize / 2, 1ull);
			glNamedBufferData(m_InstanceBuffer, m_InstanceBufferSize * sizeof(std::remove_reference_t<decltype(instanceMatrices)>::value_type), instanceMatrices.data(), GL_STATIC_DRAW);
		}
		else
		{
			glNamedBufferSubData(m_InstanceBuffer, 0, instanceMatrices.size() * sizeof(std::remove_reference_t<decltype(instanceMatrices)>::value_type), instanceMatrices.data());
		}
	}


	void InstancedRenderable::Update()
	{
		m_Meshes.clear();
		std::ranges::for_each(ModelDataSrc.MeshData, [&](auto& meshData)
		{
			m_Meshes.emplace_back(std::make_unique<InstancedMesh>(meshData, m_InstanceBuffer));
		});
	}
}
