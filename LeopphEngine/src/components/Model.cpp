#include "Model.hpp"

#include "../data/DataManager.hpp"
#include "../events/EventManager.hpp"
#include "../events/ModelCountChangedEvent.hpp"

#include <utility>

namespace leopph
{
	Model::Model(Entity& owner, const std::filesystem::path& path) :
		Component{ owner }, ResourceHandle{ path }
	{
		impl::DataManager::Register(resource);
		EventManager::Instance().Send<impl::ModelCountChangedEvent>(resource);
	}


	Model::~Model()
	{
		impl::DataManager::Unregister(resource);
		EventManager::Instance().Send<impl::ModelCountChangedEvent>(resource);
	}


	const std::filesystem::path& Model::Path() const
	{
		return resource->Path;
	}


	bool Model::CastsShadow() const
	{
		return resource->CastsShadow();
	}


	void Model::CastsShadow(const bool value)
	{
		resource->CastsShadow(value);
	}

}