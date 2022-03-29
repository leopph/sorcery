#include "Mesh.hpp"

#include "../../util/Logger.hpp"

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


	auto Mesh::Vertices() const -> std::span<Vertex const>
	{
		return m_Vertices;
	}


	auto Mesh::Vertices(std::vector<Vertex> vertices) -> void
	{
		m_Vertices = std::move(vertices);
		m_BoundingSphere = CalculateBoundingSphere();
	}


	auto Mesh::Indices() const -> std::span<unsigned const>
	{
		return m_Indices;
	}


	auto Mesh::Indices(std::vector<unsigned> indices) -> void
	{
		m_Indices = std::move(indices);
	}


	auto Mesh::Material() const -> std::shared_ptr<leopph::Material> const&
	{
		return m_Material;
	}


	auto Mesh::Material(std::shared_ptr<leopph::Material> material) -> void
	{
		if (!material)
		{
			internal::Logger::Instance().Warning("Ignoring attempt to set null Material in Mesh.");
			return;
		}

		m_Material = std::move(material);
	}


	auto Mesh::BoundingSphere() const -> Sphere const&
	{
		return m_BoundingSphere;
	}


	auto Mesh::CalculateBoundingSphere() const noexcept -> Sphere
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
