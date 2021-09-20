#include "ResourceHandleBase.hpp"

#include "../DataManager.hpp"



namespace leopph::impl
{
	ResourceHandleBase::~ResourceHandleBase() = default;


	UniqueResource* ResourceHandleBase::Find(const std::filesystem::path& path)
	{
		return DataManager::Find(path);
	}


	void ResourceHandleBase::Init(const Resource* const resource) const
	{
		DataManager::Register(resource, this);
	}


	void ResourceHandleBase::Init(const UniqueResource* const uniqueResource) const
	{
		DataManager::Register(uniqueResource, this);
	}


	void ResourceHandleBase::Deinit(const Resource* const resource) const
	{
		DataManager::Unregister(resource, this);

		if (DataManager::Count(resource) == 0)
		{
			delete resource;
		}
	}


	void ResourceHandleBase::Deinit(const UniqueResource* const uniqueResource) const
	{
		DataManager::Unregister(uniqueResource, this);

		if (DataManager::Count(uniqueResource) == 0)
		{
			delete uniqueResource;
		}
	}
}
