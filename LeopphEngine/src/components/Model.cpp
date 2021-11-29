#include "Model.hpp"

#include "../data/DataManager.hpp"

#include <utility>


namespace leopph
{
	Model::Model(leopph::Entity& owner, std::filesystem::path path) :
		Component{owner}, m_Impl{impl::DataManager::CreateOrGetModelImpl(std::move(path))}
	{
		impl::DataManager::RegisterModelComponent(m_Impl, this);
	}


	Model::~Model()
	{
		if (impl::DataManager::Models().at(*m_Impl).size() == 1ull)
		{
			impl::DataManager::DestroyModelImpl(m_Impl);
		}
		else
		{
			impl::DataManager::UnregisterModelComponent(m_Impl, this);
		}
	}


	const std::filesystem::path& Model::Path() const
	{
		return m_Impl->Path;
	}


	bool Model::CastsShadow() const
	{
		return m_Impl->CastsShadow();
	}


	void Model::CastsShadow(const bool value) const
	{
		m_Impl->CastsShadow(value);
	}

}