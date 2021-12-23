#pragma once

#include "../../rendering/geometry/GlMeshGroup.hpp"
#include "../../rendering/geometry/MeshDataGroup.hpp"

#include <functional>
#include <memory>


namespace leopph::internal
{
	class GlMeshGroupHash
	{
	public:
		using is_transparent = void;

		std::size_t operator()(const GlMeshGroup& model) const;
		std::size_t operator()(const std::shared_ptr<const MeshDataGroup>& meshDataGroup) const;


	private:
		std::hash<std::string> m_Hash;
	};
}