#include "GlMeshCollection.hpp"

#include "../../data/DataManager.hpp"
#include "../../math/Matrix.hpp"

#include <glad/glad.h>

#include <algorithm>
#include <iterator>
#include <utility>


namespace leopph::impl
{
	GlMeshCollection::GlMeshCollection(const impl::MeshDataCollection& modelData) :
		m_SharedData{std::make_shared_for_overwrite<SharedData>()}
	{
		m_SharedData->MeshDataCollection = modelData;

		glCreateBuffers(1, &m_SharedData->InstanceBuffer);
		glNamedBufferData(m_SharedData->InstanceBuffer, 2 * sizeof(Matrix4), nullptr, GL_STATIC_DRAW);

		std::ranges::for_each(modelData.Data(), [&](const auto& meshData)
		{
			m_SharedData->Meshes.emplace_back(meshData, m_SharedData->InstanceBuffer);
		});
	}

	GlMeshCollection::GlMeshCollection(const GlMeshCollection& other) :
		m_SharedData{other.m_SharedData}
	{
		++m_SharedData->HandleCount;
	}

	GlMeshCollection& GlMeshCollection::operator=(const GlMeshCollection& other) noexcept
	{
		if (&other == this)
		{
			return *this;
		}

		Deinit();
		m_SharedData = other.m_SharedData;
		++m_SharedData->HandleCount;
	}

	GlMeshCollection::GlMeshCollection(GlMeshCollection&& other) noexcept :
		GlMeshCollection{other}
	{}

	GlMeshCollection& GlMeshCollection::operator=(GlMeshCollection&& other)
	{
		return *this = other;
	}

	GlMeshCollection::~GlMeshCollection() noexcept
	{
		Deinit();
	}

	void GlMeshCollection::DrawShaded(leopph::impl::ShaderProgram& shader, const std::size_t nextFreeTextureUnit) const
	{
		for (const auto& mesh : m_SharedData->Meshes)
		{
			mesh.DrawShaded(shader, nextFreeTextureUnit, m_SharedData->RenderInstances.size());
		}
	}


	void GlMeshCollection::DrawDepth() const
	{
		for (const auto& mesh : m_SharedData->Meshes)
		{
			mesh.DrawDepth(m_SharedData->RenderInstances.size());
		}
	}

	void GlMeshCollection::UpdateInstanceGeometry() const
	{
		static std::vector<std::pair<Matrix4, Matrix4>> instanceMatrices;
		instanceMatrices.clear();
		std::ranges::transform(m_SharedData->RenderInstances, std::back_inserter(instanceMatrices), [](const auto& component)
		{
			const auto [modelMatrix, normalMatrix]{DataManager::GetMatrices(component->Entity()->Transform())};
			return std::make_pair(modelMatrix.Transposed(), normalMatrix.Transposed());
		});

		if (instanceMatrices.size() > m_SharedData->InstanceBufferSize)
		{
			m_SharedData->InstanceBufferSize *= 2;
			glNamedBufferData(m_SharedData->InstanceBuffer, m_SharedData->InstanceBufferSize * sizeof(std::remove_reference_t<decltype(instanceMatrices)>::value_type), instanceMatrices.data(), GL_DYNAMIC_DRAW);
		}
		else if (instanceMatrices.size() * 2 < m_SharedData->InstanceBufferSize)
		{
			m_SharedData->InstanceBufferSize = std::max(m_SharedData->InstanceBufferSize / 2, 1ull);
			glNamedBufferData(m_SharedData->InstanceBuffer, m_SharedData->InstanceBufferSize * sizeof(std::remove_reference_t<decltype(instanceMatrices)>::value_type), instanceMatrices.data(), GL_DYNAMIC_DRAW);
		}
		else
		{
			glNamedBufferSubData(m_SharedData->InstanceBuffer, 0, instanceMatrices.size() * sizeof(std::remove_reference_t<decltype(instanceMatrices)>::value_type), instanceMatrices.data());
		}
	}

	void GlMeshCollection::AddInstance(const RenderComponent* component) const
	{
		m_SharedData->RenderInstances.push_back(component);
	}

	void GlMeshCollection::RemoveInstance(const RenderComponent* component) const
	{
		std::erase(m_SharedData->RenderInstances, component);
	}

	const MeshDataCollection& GlMeshCollection::MeshDataCollection() const
	{
		return m_SharedData->MeshDataCollection;
	}


	void GlMeshCollection::Deinit() const
	{
		--m_SharedData->HandleCount;

		if (m_SharedData->HandleCount == 0)
		{
			glDeleteBuffers(1, &m_SharedData->InstanceBuffer);
		}
	}

}
