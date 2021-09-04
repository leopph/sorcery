#include "DataManager.hpp"

#include "../util/logger.h"

#include <stdexcept>
#include <tuple>

namespace leopph::impl
{
	std::unordered_map<Object*, std::unordered_set<Component*>, ObjectHash, ObjectEqual> DataManager::s_Objects{};

	std::unordered_set<Behavior*> DataManager::s_Behaviors{};

	std::unique_ptr<AmbientLight> DataManager::s_AmbientLight{ nullptr };

	DirectionalLight* DataManager::s_DirLight{ nullptr };

	std::unordered_set<const SpotLight*> DataManager::s_SpotLights{};
	;
	std::vector<PointLight*> DataManager::s_PointLights{};

	std::unordered_set<const ModelResource*> DataManager::s_ModelResources{};

	std::unordered_map<const Transform*, std::pair<Matrix4, Matrix4>> DataManager::s_Matrices{};

	std::list<ShadowMap> DataManager::s_ShadowMaps{};

	std::unordered_map<const Resource*, std::unordered_set<const ResourceHandleBase*>> DataManager::s_ResourcesAndHandles{};

	std::unordered_map<const UniqueResource*, std::unordered_set<const ResourceHandleBase*>, UniqueResourceHash, UniqueResourceEqual> DataManager::s_UniqueResourcesAndHandles{};

	
	void DataManager::Clear()
	{
		for (auto it = s_Objects.begin(); it != s_Objects.end();)
		{
			delete it->first;
			it = s_Objects.begin();
		}

		s_ShadowMaps.clear();

		Logger::Instance().Debug("All objects destroyed.");
	}

	
	void DataManager::Register(Object* object)
	{
		s_Objects.try_emplace(object);
	}


	void DataManager::Unregister(Object* object)
	{
		s_Objects.erase(object);
	}


	Object* DataManager::Find(const std::string& name)
	{
		const auto it = s_Objects.find(name);
		return it != s_Objects.end() ? it->first : nullptr;
	}


	const std::unordered_map<Object*, std::unordered_set<Component*>, ObjectHash, ObjectEqual>& DataManager::Objects()
	{
		return s_Objects;
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


	const std::unordered_set<Component*>& DataManager::Components(Object* object)
	{
		return s_Objects[object];
	}


	void DataManager::Register(Component* component)
	{
		s_Objects[&component->object].insert(component);
	}


	void DataManager::Unregister(Component* component)
	{
		s_Objects[&component->object].erase(component);
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
			if (*it == pointLight)
			{
				s_PointLights.erase(it);
				return;
			}
	}


	leopph::AmbientLight* DataManager::AmbientLight()
	{
		return s_AmbientLight.get();
	}

	void DataManager::AmbientLight(leopph::AmbientLight*&& light)
	{
		s_AmbientLight = std::unique_ptr<leopph::AmbientLight>(std::forward<leopph::AmbientLight*>(light));
	}


	const std::list<ShadowMap>& DataManager::ShadowMaps()
	{
		return s_ShadowMaps;
	}


	void DataManager::CreateShadowMap(const Vector2& resolution)
	{
		s_ShadowMaps.emplace_back(resolution);
	}


	void DataManager::DeleteShadowMap()
	{
		if (!s_ShadowMaps.empty())
		{
			s_ShadowMaps.pop_back();
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


	const std::unordered_set<const ModelResource*> DataManager::Models()
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


	std::unordered_set<const ResourceHandleBase*> DataManager::ModelComponents(const ModelResource* model)
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