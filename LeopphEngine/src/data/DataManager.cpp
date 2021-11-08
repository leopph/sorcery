#include "DataManager.hpp"

#include "../util/logger.h"



namespace leopph::impl
{
	std::unordered_map<Entity*, std::unordered_set<Component*>, EntityHash, EntityEqual> DataManager::s_EntitiesAndComponents{};

	std::unordered_set<Behavior*> DataManager::s_Behaviors{};

	DirectionalLight* DataManager::s_DirLight{nullptr};

	std::unordered_set<const SpotLight*> DataManager::s_SpotLights{};
	;
	std::vector<PointLight*> DataManager::s_PointLights{};

	std::unordered_set<const ModelResource*> DataManager::s_ModelResources{};

	std::unordered_map<const Transform*, std::pair<Matrix4, Matrix4>> DataManager::s_Matrices{};

	std::unordered_map<const Resource*, std::unordered_set<const ResourceHandleBase*>> DataManager::s_ResourcesAndHandles{};

	std::unordered_map<const UniqueResource*, std::unordered_set<const ResourceHandleBase*>, UniqueResourceHash, UniqueResourceEqual> DataManager::s_UniqueResourcesAndHandles{};


	void DataManager::Clear()
	{
		for (auto it = s_EntitiesAndComponents.begin(); it != s_EntitiesAndComponents.end();)
		{
			delete it->first;
			it = s_EntitiesAndComponents.begin();
		}

		Logger::Instance().Debug("All data have been cleared.");
	}


	void DataManager::Register(Entity* entity)
	{
		s_EntitiesAndComponents.try_emplace(entity);
	}


	void DataManager::Unregister(Entity* entity)
	{
		s_EntitiesAndComponents.erase(entity);
	}


	Entity* DataManager::Find(const std::string& name)
	{
		const auto it = s_EntitiesAndComponents.find(name);
		return it != s_EntitiesAndComponents.end() ? it->first : nullptr;
	}


	const std::unordered_map<Entity*, std::unordered_set<Component*>, EntityHash, EntityEqual>& DataManager::EntitiesAndComponents()
	{
		return s_EntitiesAndComponents;
	}


	const std::unordered_set<Behavior*>& DataManager::Behaviors()
	{
		return s_Behaviors;
	}


	void DataManager::Register(Behavior* behavior)
	{
		s_Behaviors.insert(behavior);
	}


	void DataManager::Unregister(Behavior* behavior)
	{
		s_Behaviors.erase(behavior);
	}


	const std::unordered_set<Component*>& DataManager::Components(Entity* entity)
	{
		return s_EntitiesAndComponents[entity];
	}


	void DataManager::Register(Component* component)
	{
		s_EntitiesAndComponents[&component->Entity].insert(component);
	}


	void DataManager::Unregister(Component* component)
	{
		s_EntitiesAndComponents[&component->Entity].erase(component);
	}


	DirectionalLight* DataManager::DirectionalLight()
	{
		return s_DirLight;
	}


	void DataManager::DirectionalLight(leopph::DirectionalLight* dirLight)
	{
		s_DirLight = dirLight;
	}


	const std::vector<PointLight*>& DataManager::PointLights()
	{
		return s_PointLights;
	}


	void DataManager::Register(PointLight* pointLight)
	{
		s_PointLights.push_back(pointLight);
	}


	void DataManager::Unregister(PointLight* pointLight)
	{
		for (auto it = s_PointLights.begin(); it != s_PointLights.end(); ++it)
		{
			if (*it == pointLight)
			{
				s_PointLights.erase(it);
				return;
			}
		}
	}


	const std::unordered_set<const SpotLight*>& DataManager::SpotLights()
	{
		return s_SpotLights;
	}


	void DataManager::Register(const SpotLight* spotLight)
	{
		s_SpotLights.emplace(spotLight);
	}


	void DataManager::Unregister(const SpotLight* spotLight)
	{
		s_SpotLights.erase(spotLight);
	}


	void DataManager::Register(const Resource* resource)
	{
		s_ResourcesAndHandles.try_emplace(resource);
	}


	void DataManager::Register(const UniqueResource* resource)
	{
		s_UniqueResourcesAndHandles.try_emplace(resource);
	}


	void DataManager::Unregister(const Resource* resource)
	{
		s_ResourcesAndHandles.erase(resource);
	}


	void DataManager::Unregister(const UniqueResource* resource)
	{
		s_UniqueResourcesAndHandles.erase(resource);
	}


	void DataManager::Register(const Resource* resource, const ResourceHandleBase* handle)
	{
		s_ResourcesAndHandles.at(resource).insert(handle);
	}


	void DataManager::Register(const UniqueResource* resource, const ResourceHandleBase* handle)
	{
		s_UniqueResourcesAndHandles.at(resource).insert(handle);
	}


	void DataManager::Unregister(const Resource* resource, const ResourceHandleBase* handle)
	{
		s_ResourcesAndHandles.at(resource).erase(handle);
	}


	void DataManager::Unregister(const UniqueResource* resource, const ResourceHandleBase* handle)
	{
		s_UniqueResourcesAndHandles.at(resource).erase(handle);
	}


	std::size_t DataManager::Count(const Resource* resource)
	{
		return s_ResourcesAndHandles.at(resource).size();
	}


	std::size_t DataManager::Count(const UniqueResource* resource)
	{
		return s_UniqueResourcesAndHandles.at(resource).size();
	}


	UniqueResource* DataManager::Find(const std::filesystem::path& path)
	{
		auto it = s_UniqueResourcesAndHandles.find(path);
		return it == s_UniqueResourcesAndHandles.end() ? nullptr : const_cast<UniqueResource*>(it->first);
	}


	const std::unordered_set<const ModelResource*>& DataManager::Models()
	{
		return s_ModelResources;
	}


	void DataManager::Register(const ModelResource* model)
	{
		s_ModelResources.insert(model);
	}


	void DataManager::Unregister(const ModelResource* model)
	{
		s_ModelResources.erase(model);
	}


	const std::unordered_set<const ResourceHandleBase*>& DataManager::ModelComponents(const ModelResource* model)
	{
		return s_UniqueResourcesAndHandles.at(model);
	}


	void DataManager::StoreMatrices(const Transform* transform, const Matrix4& model, const Matrix4& normal)
	{
		s_Matrices.insert_or_assign(transform, std::pair<Matrix4, Matrix4>{model, normal});
	}


	void DataManager::DiscardMatrices(const Transform* transform)
	{
		s_Matrices.erase(transform);
	}


	const std::pair<Matrix4, Matrix4>& DataManager::GetMatrices(const Transform* transform)
	{
		return s_Matrices.at(transform);
	}
}
