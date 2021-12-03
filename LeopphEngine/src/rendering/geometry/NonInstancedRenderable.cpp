#include "NonInstancedRenderable.hpp"


namespace leopph::impl
{
	NonInstancedRenderable::NonInstancedRenderable(ModelData& modelData) :
		Renderable{modelData},
		m_Meshes{}
	{
		std::ranges::for_each(modelData.MeshData, [&](auto& meshData)
		{
			m_Meshes.emplace_back(std::make_unique<Mesh>(meshData));
		});
	}


	void NonInstancedRenderable::DrawShaded(leopph::impl::ShaderProgram& shader, const std::size_t nextFreeTextureUnit) const
	{
		for (const auto& mesh : m_Meshes)
		{
			mesh->DrawShaded(shader, nextFreeTextureUnit);
		}
	}


	void NonInstancedRenderable::DrawDepth() const
	{
		for (const auto& mesh : m_Meshes)
		{
			mesh->DrawDepth();
		}
	}


	void NonInstancedRenderable::Update()
	{
		m_Meshes.clear();
		std::ranges::for_each(ModelDataSrc.MeshData, [&](auto& meshData)
		{
			m_Meshes.emplace_back(std::make_unique<Mesh>(meshData));
		});
	}
}