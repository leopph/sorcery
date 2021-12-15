#include "DataManager.hpp"

#include "../util/logger.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>


namespace leopph::impl
{
	std::unordered_map<Entity*, std::unordered_set<std::unique_ptr<Component>, PointerHash, PointerEqual>, EntityHash, EntityEqual> DataManager::s_EntitiesAndComponents{};
	std::unordered_set<Behavior*> DataManager::s_Behaviors{};
	DirectionalLight* DataManager::s_DirLight{nullptr};
	std::unordered_set<const SpotLight*> DataManager::s_SpotLights{};
	std::vector<PointLight*> DataManager::s_PointLights{};
	std::unordered_map<const Transform*, std::pair<Matrix4, Matrix4>> DataManager::s_Matrices{};
	std::vector<Texture*> DataManager::s_Textures{};
	std::unordered_map<SkyboxImpl, std::unordered_set<Skybox*>, PathedHash<SkyboxImpl>, PathedEqual<SkyboxImpl>> DataManager::s_Skyboxes{};
	std::unordered_set<FileModelData, PathedHash<FileModelData>, PathedEqual<FileModelData>> DataManager::s_FileModelData{};
	std::unordered_map<InstancedRenderable, std::unordered_set<InstancedRenderComponent*>, RenderableHash, RenderableEqual> DataManager::s_InstancedRenderables{};
	std::unordered_map<std::unique_ptr<NonInstancedRenderable>, NonInstancedRenderComponent*, PointerHash, PointerEqual> DataManager::s_NonInstancedRenderables{};


	void DataManager::Clear()
	{
		while (!s_EntitiesAndComponents.empty())
		{
			auto node{s_EntitiesAndComponents.extract(s_EntitiesAndComponents.begin())};
			delete node.key();
			node.mapped().clear();
		}

		s_FileModelData.clear();
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


	const std::unordered_map<Entity*, std::unordered_set<std::unique_ptr<Component>, PointerHash, PointerEqual>, EntityHash, EntityEqual>& DataManager::EntitiesAndComponents()
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


	const std::unordered_set<std::unique_ptr<Component>, PointerHash, PointerEqual>& DataManager::ComponentsOfEntity(const Entity* entity)
	{
		if (const auto it{s_EntitiesAndComponents.find(entity)};
			it != s_EntitiesAndComponents.end())
		{
			return it->second;
		}

		const auto msg{"Entity at address [" + std::to_string(reinterpret_cast<unsigned long long>(entity)) + "] was not found while trying access its components."};
		Logger::Instance().Error(msg);
		throw std::out_of_range{msg};
	}


	void DataManager::RegisterComponentForEntity(const Entity* entity, std::unique_ptr<Component>&& component)
	{
		if (const auto it{s_EntitiesAndComponents.find(entity)};
			it != s_EntitiesAndComponents.end())
		{
			it->second.emplace(std::move(component));
		}
		else
		{
			const auto msg{"Entity at address [" + std::to_string(reinterpret_cast<unsigned long long>(entity)) + "] was not found while trying to register component at address [" + std::to_string(reinterpret_cast<unsigned long long>(component.get())) + "] to it."};
			Logger::Instance().Error(msg);
			throw std::out_of_range{msg};
		}
	}


	void DataManager::UnregisterComponentFromEntity(const Entity* entity, const Component* component)
	{
		if (const auto entityIt{s_EntitiesAndComponents.find(entity)};
			entityIt != s_EntitiesAndComponents.end())
		{
			if (const auto componentIt{entityIt->second.find(component)};
				componentIt != entityIt->second.end())
			{
				entityIt->second.erase(componentIt);
			}
			else
			{
				const auto msg{"Component at address [" + std::to_string(reinterpret_cast<unsigned long long>(component)) + "] was not found while trying to unregister it from Entity at address [" + std::to_string(reinterpret_cast<unsigned long long>(entity)) + "]."};
				Logger::Instance().Error(msg);
				throw std::out_of_range{msg};
			}
		}
		else
		{
			const auto msg{"Entity at address [" + std::to_string(reinterpret_cast<unsigned long long>(entity)) + "] was not found while trying to unregister component at address [" + std::to_string(reinterpret_cast<unsigned long long>(component)) + "] from it."};
			Logger::Instance().Error(msg);
			throw std::out_of_range{msg};
		}
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

	InstancedRenderable& DataManager::CreateOrGetInstancedRenderable(ModelData& modelData)
	{
		if (const auto it{s_InstancedRenderables.find(modelData)};
			it != s_InstancedRenderables.end())
		{
			return const_cast<InstancedRenderable&>(it->first);
		}

		return const_cast<InstancedRenderable&>(s_InstancedRenderables.emplace(modelData, std::unordered_set<InstancedRenderComponent*>{}).first->first);
	}

	void DataManager::DestroyInstancedRenderable(const InstancedRenderable& renderable)
	{
		s_InstancedRenderables.erase(renderable);
	}

	void DataManager::RegisterInstancedRenderComponent(const InstancedRenderable& renderable, InstancedRenderComponent* const component)
	{
		s_InstancedRenderables.at(renderable).insert(component);
	}

	void DataManager::UnregisterInstancedRenderComponent(const InstancedRenderable& renderable, InstancedRenderComponent* const component)
	{
		s_InstancedRenderables.at(renderable).erase(component);
	}

	const std::unordered_map<InstancedRenderable, std::unordered_set<InstancedRenderComponent*>, RenderableHash, RenderableEqual>& DataManager::InstancedRenderables()
	{
		return s_InstancedRenderables;
	}

	void DataManager::RegisterTexture(Texture* const texture)
	{
		s_Textures.push_back(texture);
		SortTextures();
	}

	void DataManager::UnregisterTexture(Texture* const texture)
	{
		std::erase(s_Textures, texture);
		SortTextures();
	}

	std::shared_ptr<Texture> DataManager::FindTexture(const std::filesystem::path& path)
	{
		if (const auto it{
				std::ranges::lower_bound(s_Textures, path, [](const auto& elemPath, const auto& valPath)
				                         {
					                         return elemPath.compare(valPath);
				                         }, [](const auto& texture) -> const auto&
				                         {
					                         return texture->Path;
				                         })
			};
			it != s_Textures.end())
		{
			return (*it)->shared_from_this();
		}
		return nullptr;
	}

	void DataManager::SortTextures()
	{
		std::ranges::sort(s_Textures, [](const auto& left, const auto& right)
		{
			return left->Path.compare(right->Path);
		});
	}

	SkyboxImpl* DataManager::CreateOrGetSkyboxImpl(std::filesystem::path allPaths)
	{
		if (const auto it{s_Skyboxes.find(allPaths)};
			it != s_Skyboxes.end())
		{
			return &const_cast<SkyboxImpl&>(it->first);
		}
		return &const_cast<SkyboxImpl&>(s_Skyboxes.emplace(std::move(allPaths), std::unordered_set<Skybox*>{}).first->first);
	}

	void DataManager::DestroySkyboxImpl(SkyboxImpl* const skybox)
	{
		s_Skyboxes.erase(*skybox);
	}

	void DataManager::RegisterSkyboxHandle(SkyboxImpl* const skybox, Skybox* const handle)
	{
		s_Skyboxes.at(*skybox).insert(handle);
	}

	void DataManager::UnregisterSkyboxHandle(SkyboxImpl* const skybox, Skybox* const handle)
	{
		s_Skyboxes.at(*skybox).erase(handle);
	}

	const std::unordered_map<SkyboxImpl, std::unordered_set<Skybox*>, PathedHash<SkyboxImpl>, PathedEqual<SkyboxImpl>>& DataManager::Skyboxes()
	{
		return s_Skyboxes;
	}

	FileModelData& DataManager::LoadOrGetFileModelData(std::filesystem::path path)
	{
		if (const auto it{s_FileModelData.find(path)};
			it != s_FileModelData.end())
		{
			return const_cast<FileModelData&>(*it);
		}

		return const_cast<FileModelData&>(*s_FileModelData.emplace(std::move(path)).first);
	}

	NonInstancedRenderable& DataManager::CreateNonInstancedRenderable(ModelData& modelData, NonInstancedRenderComponent* const component)
	{
		return *s_NonInstancedRenderables.emplace(new NonInstancedRenderable{modelData}, component).first->first;
	}

	void DataManager::DestroyNonInstancedRenderable(const NonInstancedRenderable& renderable)
	{
		s_NonInstancedRenderables.erase(s_NonInstancedRenderables.find(&renderable));
	}

	const std::unordered_map<std::unique_ptr<NonInstancedRenderable>, NonInstancedRenderComponent*, PointerHash, PointerEqual>& DataManager::NonInstancedRenderables()
	{
		return s_NonInstancedRenderables;
	}
}
