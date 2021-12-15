#include "Model.hpp"

#include "../../data/DataManager.hpp"

#include <utility>


namespace leopph
{
	Model::Model(leopph::Entity* const entity, std::filesystem::path path) :
		impl::NonInstancedRenderComponent{entity, impl::DataManager::LoadOrGetFileModelData(path)},
		Path{std::move(path)}
	{}

	bool Model::CastsShadow() const
	{
		return m_Impl.CastsShadow();
	}

	void Model::CastsShadow(bool value)
	{
		m_Impl.CastsShadow(value);
	}
}
