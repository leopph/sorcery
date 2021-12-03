#pragma once

#include "../../rendering/geometry/ModelData.hpp"
#include "../../rendering/geometry/Renderable.hpp"


namespace leopph::impl
{
	class RenderableEqual
	{
	public:
		using is_transparent = void;

		bool operator()(const Renderable& left, const Renderable& right) const;
		bool operator()(const Renderable& left, const ModelData& right) const;
		bool operator()(const ModelData& left, const Renderable& right) const;
	};
}