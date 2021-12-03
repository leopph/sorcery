#include "RenderableEqual.hpp"


namespace leopph::impl
{
	bool RenderableEqual::operator()(const Renderable& left, const Renderable& right) const
	{
		return &left.ModelDataSrc == &right.ModelDataSrc;
	}

	bool RenderableEqual::operator()(const Renderable& left, const ModelData& right) const
	{
		return &left.ModelDataSrc == &right;
	}

	bool RenderableEqual::operator()(const ModelData& left, const Renderable& right) const
	{
		return &left == &right.ModelDataSrc;
	}
}