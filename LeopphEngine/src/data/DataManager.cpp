#include "DataManager.hpp"

#include "../util/logger.h"

#include <stdexcept>
#include <tuple>

namespace leopph::impl
{
	std::unique_ptr<AmbientLight> DataManager::s_AmbientLight{ nullptr };

	DirectionalLight* DataManager::s_DirLight{ nullptr };

	std::vector<PointLight*> DataManager::s_PointLights{};

	std::unordered_set<const SpotLight*> DataManager::s_SpotLights{};

	std::map<Object*, std::set<Component*>, ObjectLess> DataManager::s_Objects{}
	;
	std::set<Behavior*> DataManager::s_Behaviors{};

	std::list<ShadowMap> DataManager::s_ShadowMaps{};

	std::unordered_map<const Object*, std::pair<const Matrix4, const Matrix4>> DataManager::s_MatrixCache{};

	std::unordered_map<unsigned, std::size_t> DataManager::s_Buffers{};

	std::unordered_map<const Resource*, std::unordered_set<const ResourceHandleBase*>> DataManager::s_ResourcesAndHandles{};

	std::unordered_map<const UniqueResource*, std::unordered_set<const ResourceHandleBase*>, UniqueResourceHash, UniqueResourceEqual> DataManager::s_UniqueResourcesAndHandles{};

	std::unordered_set<const ModelResource*> DataManager::s_ModelResources{};

	
	void DataManager::DestroyAllObjects()
	{
		for (auto it = s_Objects.begin(); it != s_Objects.end();)
		{
			delete it->first;
			it = s_Objects.begin();
		}

		s_ShadowMaps.clear();

		Logger::Instance().Debug("All objects destroyed.");
	}

	
	void DataManager::RegisterObject(Object* object)
	{
		s_Objects.try_emplace(object);
	}


	void DataManager::UnregisterObject(Object* object)
	{
		s_Objects.erase(object);
		s_MatrixCache.erase(object);
	}


	Object* DataManager::FindObject(const std::string& name)
	{
		const auto it = s_Objects.find(name);
		return it != s_Objects.end() ? it->first : nullptr;
	}


	const std::map<Object*, std::set<Component*>, ObjectLess>& DataManager::Objects()
	{
		return s_Objects;
	}


	const std::set<Behavior*>& DataManager::Behaviors()
	{
		return s_Behaviors;
	}


	void DataManager::RegisterBehavior(Behavior* behavior)
	{
		s_Behaviors.insert(behavior);
	}


	void DataManager::UnregisterBehavior(Behavior* behavior)
	{
		s_Behaviors.erase(behavior);
	}


	const std::set<Component*>& DataManager::Components(Object* object)
	{
		return s_Objects[object];
	}


	void DataManager::RegisterComponent(Component* component)
	{
		s_Objects[&component->object].insert(component);
	}


	void DataManager::UnregisterComponent(Component* component)
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


	void DataManager::RegisterPointLight(PointLight* pointLight)
	{
		s_PointLights.push_back(pointLight);
	}


	void DataManager::UnregisterPointLight(PointLight* pointLight)
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


	const std::pair<const Matrix4, const Matrix4>& DataManager::ModelAndNormalMatrices(const Object* const object)
	{
		if (!object->isStatic)
		{
			const auto msg{ "Trying to access cached model matrix for dynamic object [" + object->name + "]." };
			Logger::Instance().Warning(msg);
			throw std::runtime_error{ msg };
		}

		if (auto it = s_MatrixCache.find(object); it != s_MatrixCache.end())
			return it->second;

		Matrix4 modelMatrix = Matrix4::Scale(object->Transform().Scale());
		modelMatrix *= static_cast<Matrix4>(object->Transform().Rotation());
		modelMatrix *= Matrix4::Translate(object->Transform().Position());

		return s_MatrixCache.emplace(object, std::make_pair(modelMatrix, modelMatrix.Inverse().Transposed())).first->second;
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


	void DataManager::RegisterSpotLight(const SpotLight* spotLight)
	{
		s_SpotLights.emplace(spotLight);
	}


	void DataManager::UnregisterSpotLight(const SpotLight* spotLight)
	{
		s_SpotLights.erase(spotLight);
	}


	void DataManager::RegisterBuffer(const RefCountedBuffer& buffer)
	{
		s_Buffers.try_emplace(buffer.name, 0);
		s_Buffers[buffer.name]++;
	}


	void DataManager::UnregisterBuffer(const RefCountedBuffer& buffer)
	{
		if (s_Buffers.contains(buffer.name))
		{
			s_Buffers[buffer.name]--;
		}
		else
		{
			Logger::Instance().Warning("Trying to unregister buffer [" + std::to_string(buffer.name) + "] but it is not registered.");
		}
	}


	std::size_t DataManager::ReferenceCount(const RefCountedBuffer& buffer)
	{
		const auto it{ s_Buffers.find(buffer.name) };

		if (it == s_Buffers.end())
		{
			return 0;
		}

		return it->second;
	}


	void DataManager::RegisterResource(const Resource* resource)
	{
		s_ResourcesAndHandles.try_emplace(resource);
	}


	void DataManager::RegisterResource(const UniqueResource* resource)
	{
		s_UniqueResourcesAndHandles.try_emplace(resource);
	}


	void DataManager::UnregisterResource(const Resource* resource)
	{
		s_ResourcesAndHandles.erase(resource);
	}


	void DataManager::UnregisterResource(const UniqueResource* resource)
	{
		s_UniqueResourcesAndHandles.erase(resource);
	}


	void DataManager::RegisterResourceHandle(const Resource* resource, const ResourceHandleBase* handle)
	{
		s_ResourcesAndHandles.at(resource).insert(handle);
	}


	void DataManager::RegisterResourceHandle(const UniqueResource* resource, const ResourceHandleBase* handle)
	{
		s_UniqueResourcesAndHandles.at(resource).insert(handle);
	}


	void DataManager::UnregisterResourceHandle(const Resource* resource, const ResourceHandleBase* handle)
	{
		s_ResourcesAndHandles.at(resource).erase(handle);
	}


	void DataManager::UnregisterResourceHandle(const UniqueResource* resource, const ResourceHandleBase* handle)
	{
		s_UniqueResourcesAndHandles.at(resource).erase(handle);
	}


	std::size_t DataManager::ResourceHandleCount(const Resource* resource)
	{
		return s_ResourcesAndHandles.at(resource).size();
	}


	std::size_t DataManager::ResourceHandleCount(const UniqueResource* resource)
	{
		return s_UniqueResourcesAndHandles.at(resource).size();
	}


	UniqueResource* DataManager::FindUniqueResource(const std::filesystem::path& path)
	{
		auto it = s_UniqueResourcesAndHandles.find(path);
		return it == s_UniqueResourcesAndHandles.end() ? nullptr : const_cast<UniqueResource*>(it->first);
	}


	const std::unordered_set<const ModelResource*> DataManager::Models()
	{
		return s_ModelResources;
	}


	void DataManager::RegisterModel(const ModelResource* model)
	{
		s_ModelResources.insert(model);
	}


	void DataManager::UnregisterModel(const ModelResource* model)
	{
		s_ModelResources.erase(model);
	}


	std::unordered_set<const ResourceHandleBase*> DataManager::ModelComponents(const ModelResource* model)
	{
		return s_UniqueResourcesAndHandles.at(model);
	}
}