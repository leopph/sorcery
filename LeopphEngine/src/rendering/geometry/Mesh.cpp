#include "Mesh.hpp"

#include "../../util/Logger.hpp"

#include <algorithm>
#include <stdexcept>


namespace leopph
{
	internal::Mesh::Mesh(std::span<const Vertex> vertices, std::span<const unsigned> indices, std::shared_ptr<leopph::Material> material) :
		m_Vertices{vertices.begin(), vertices.end()},
		m_Indices{indices.begin(), indices.end()},
		m_Material{std::move(material)},
		m_BoundingSphere{CalculateBoundingSphere()}
	{
		if (!m_Material)
		{
			const auto errMsg{"Trying to set null Material in Mesh."};
			Logger::Instance().Error(errMsg);
			throw std::invalid_argument{errMsg};
		}
	}


	auto internal::Mesh::Vertices() const -> std::span<const Vertex>
	{
		return m_Vertices;
	}


	auto internal::Mesh::Vertices(std::span<Vertex> vertices) -> void
	{
		m_Vertices.assign(vertices.begin(), vertices.end());
		m_BoundingSphere = CalculateBoundingSphere();
	}


	auto internal::Mesh::Indices() const -> std::span<const unsigned>
	{
		return m_Indices;
	}


	auto internal::Mesh::Indices(std::span<unsigned> indices) -> void
	{
		m_Indices.assign(indices.begin(), indices.end());
	}


	auto internal::Mesh::Material() const -> const std::shared_ptr<leopph::Material>&
	{
		return m_Material;
	}


	auto internal::Mesh::Material(std::shared_ptr<leopph::Material> material) -> void
	{
		if (!material)
		{
			Logger::Instance().Warning("Ignoring attempt to set null Material in Mesh.");
			return;
		}

		m_Material = std::move(material);
	}


	auto internal::Mesh::BoundingSphere() const -> const Sphere&
	{
		return m_BoundingSphere;
	}


	auto internal::Mesh::CalculateBoundingSphere() const noexcept -> Sphere
	{
		if (m_Vertices.empty())
		{
			return {};
		}

		return Sphere
		{
			.Origin = Vector3{},
			.Radius = std::ranges::max(m_Vertices, std::ranges::less{}, [](const Vertex& vertex)
			{
				return vertex.Position.Length();
			}).Position.Length()
		};
	}
}
