#include "InstancedModel.hpp"

#include "../../data/DataManager.hpp"

#include <utility>


namespace leopph
{
	InstancedModel::InstancedModel(leopph::Entity& owner, std::filesystem::path path) :
		impl::RenderComponent{owner}, Path{path}, m_Impl{&impl::DataManager::CreateOrGetInstancedModelImpl(impl::DataManager::LoadOrGetFileModelData(std::move(path)))}
	{
		impl::DataManager::RegisterModelComponent(*m_Impl, this);
	}


	InstancedModel::~InstancedModel()
	{
		if (impl::DataManager::InstancedModels().at(*m_Impl).size() == 1ull)
		{
			impl::DataManager::DestroyInstancedModelImpl(*m_Impl);
		}
		else
		{
			impl::DataManager::UnregisterModelComponent(*m_Impl, this);
		}
	}


	bool InstancedModel::CastsShadow() const
	{
		return m_Impl->CastsShadow();
	}


	void InstancedModel::CastsShadow(bool value)
	{
		m_Impl->CastsShadow(value);
	}
}