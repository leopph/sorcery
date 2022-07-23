#include "Mesh.hpp"

#include "Logger.hpp"

#include <algorithm>
#include <stdexcept>


namespace leopph
{
	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned> indices, std::shared_ptr<leopph::Material> material) :
		m_Vertices{std::move(vertices)},
		m_Indices{std::move(indices)},
		m_Material{std::move(material)},
		m_BoundingSphere{CalculateBoundingSphere()}
	{
		if (!m_Material)
		{
			auto const errMsg{"Trying to set null Material in Mesh."};
			internal::Logger::Instance().Error(errMsg);
			throw std::invalid_argument{errMsg};
		}
	}



	std::span<Vertex const> Mesh::Vertices() const
	{
		return m_Vertices;
	}



	void Mesh::Vertices(std::vector<Vertex> vertices)
	{
		m_Vertices = std::move(vertices);
		m_BoundingSphere = CalculateBoundingSphere();
	}



	std::span<unsigned const> Mesh::Indices() const
	{
		return m_Indices;
	}



	void Mesh::Indices(std::vector<unsigned> indices)
	{
		m_Indices = std::move(indices);
	}



	std::shared_ptr<leopph::Material> const& Mesh::Material() const
	{
		return m_Material;
	}



	void Mesh::Material(std::shared_ptr<leopph::Material> material)
	{
		if (!material)
		{
			internal::Logger::Instance().Warning("Ignoring attempt to set null Material in Mesh.");
			return;
		}

		m_Material = std::move(material);
	}



	Sphere const& Mesh::BoundingSphere() const
	{
		return m_BoundingSphere;
	}



	Sphere Mesh::CalculateBoundingSphere() const noexcept
	{
		if (m_Vertices.empty())
		{
			return {};
		}

		return Sphere
		{
			.Origin = Vector3{},
			.Radius = std::ranges::max(m_Vertices, std::ranges::less{}, [](Vertex const& vertex)
			{
				return vertex.Position.Length();
			}).Position.Length()
		};
	}
}
