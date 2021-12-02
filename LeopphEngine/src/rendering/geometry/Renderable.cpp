#include "Renderable.hpp"

#include "../../data/DataManager.hpp"


namespace leopph::impl
{
	Renderable::Renderable(ModelData& modelData) :
		ModelDataSrc{modelData}
	{
		DataManager::RegisterRenderable(this);
	}

	Renderable::~Renderable()
	{
		DataManager::UnregisterRenderable(this);
	}

	bool Renderable::CastsShadow() const
	{
		return m_CastsShadow;
	}

	void Renderable::CastsShadow(const bool value)
	{
		m_CastsShadow = value;
	}
}