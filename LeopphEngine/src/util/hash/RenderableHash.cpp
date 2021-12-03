#include "RenderableHash.hpp"


namespace leopph::impl
{
	std::size_t RenderableHash::operator()(const Renderable& model) const
	{
		return m_Hash(& model.ModelDataSrc);
	}

	std::size_t RenderableHash::operator()(const ModelData& modelData) const
	{
		return m_Hash(&modelData);
	}
}