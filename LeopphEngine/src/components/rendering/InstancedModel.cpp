#include "InstancedModel.hpp"

#include "../../data/DataManager.hpp"

#include <utility>


namespace leopph
{
	InstancedModel::InstancedModel(leopph::Entity* const entity, std::filesystem::path path) :
		impl::InstancedRenderComponent{entity, impl::DataManager::LoadOrGetFileModelData(path)},
		Path{std::move(path)}
	{}


	bool InstancedModel::CastsShadow() const
	{
		return m_Impl.CastsShadow();
	}


	void InstancedModel::CastsShadow(bool value)
	{
		m_Impl.CastsShadow(value);
	}
}
