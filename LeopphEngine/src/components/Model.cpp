#include "Model.hpp"

#include "../data/DataManager.hpp"
#include "../events/ModelInstanceCountEvent.hpp"
#include "../events/handling/EventManager.hpp"


namespace leopph
{
	Model::Model(Entity& owner, const std::filesystem::path& path) :
		Component{ owner }, ResourceHandle{ path }
	{
		impl::DataManager::Register(resource);
		EventManager::Instance().Send<impl::ModelInstanceCountEvent>(resource);
	}


	Model::~Model()
	{
		impl::DataManager::Unregister(resource);
		EventManager::Instance().Send<impl::ModelInstanceCountEvent>(resource);
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