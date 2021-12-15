#pragma once

#include "../../rendering/geometry/GlMeshCollection.hpp"
#include "../../rendering/geometry/MeshDataCollection.hpp"

#include <functional>


namespace leopph::impl
{
	class RenderableHash
	{
	public:
		using is_transparent = void;

		std::size_t operator()(const GlMeshCollection& model) const;
		std::size_t operator()(const MeshDataCollection& modelData) const;


	private:
		std::hash<std::string> m_Hash;
	};
}