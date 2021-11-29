#include "Model.hpp"

#include "../data/DataManager.hpp"


namespace leopph
{
	Model::Model(leopph::Entity& owner, const std::filesystem::path& path) :
		Component{ owner }, ResourceHandle{ path }
	{
		impl::DataManager::Register(resource);
	}


	Model::~Model()
	{
		impl::DataManager::Unregister(resource);
	}


	const std::filesystem::path& Model::Path() const
	{
		return resource->Path;
	}


	bool Model::CastsShadow() const
	{
		return resource->CastsShadow();
	}


	void Model::CastsShadow(const bool value) const
	{
		resource->CastsShadow(value);
	}

}