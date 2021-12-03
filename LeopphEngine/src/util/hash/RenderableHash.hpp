#pragma once

#include "../../rendering/geometry/ModelData.hpp"
#include "../../rendering/geometry/Renderable.hpp"

#include <functional>
#include <cstddef>


namespace leopph::impl
{
	class RenderableHash
	{
	public:
		using is_transparent = void;

		std::size_t operator()(const Renderable& model) const;
		std::size_t operator()(const ModelData& modelData) const;


	private:
		std::hash<const ModelData*> m_Hash;
	};
}