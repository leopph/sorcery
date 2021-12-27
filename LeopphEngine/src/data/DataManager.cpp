#include "DataManager.hpp"

#include "../util/logger.h"

#include <stdexcept>


namespace leopph::internal
{
	auto DataManager::Instance() -> DataManager&
	{
		static DataManager instance;
		return instance;
	}


	auto DataManager::Clear() -> void
	{
		m_EntitiesAndComponents.clear();
		// All containers should be empty at this point.
	}


	// BEHAVIORS

	auto DataManager::RegisterBehavior(Behavior* behavior) -> void
	{
		m_Behaviors.push_back(behavior);
	}


	auto DataManager::UnregisterBehavior(const Behavior* behavior) -> void
	{
		std::erase(m_Behaviors, behavior);
	}


	// ENTITIES

	auto DataManager::StoreEntity(std::unique_ptr<Entity> entity) -> void
	{
		m_EntitiesAndComponents.emplace_back(std::move(entity));
		SortEntities();
	}


	auto DataManager::DestroyEntity(const Entity* entity) -> void
	{
		std::erase_if(m_EntitiesAndComponents, [&](const auto& elem)
		{
			return elem.Entity.get() == entity;
		});
		SortEntities();
	}


	auto DataManager::FindEntityInternal(const std::string& name) -> decltype(m_EntitiesAndComponents)::iterator
	{
		return FindEntityInternalCommon(this, name);
	}


	auto DataManager::FindEntityInternal(const std::string& name) const -> decltype(m_EntitiesAndComponents)::const_iterator
	{
		return FindEntityInternalCommon(this, name);
	}


	auto DataManager::FindEntity(const std::string& name) -> Entity*
	{
		const auto it = FindEntityInternal(name);
		return it != m_EntitiesAndComponents.end() ? it->Entity.get() : nullptr;
	}


	auto DataManager::RegisterComponentForEntity(const Entity* entity, std::unique_ptr<Component>&& component) -> void
	{
		if (const auto it{FindEntityInternal(entity->Name())};
			it != m_EntitiesAndComponents.end())
		{
			it->Components.emplace_back(std::move(component));
		}
		else
		{
			const auto msg{"Entity at address [" + std::to_string(reinterpret_cast<unsigned long long>(entity)) + "] was not found while trying to register component at address [" + std::to_string(reinterpret_cast<unsigned long long>(component.get())) + "] to it."};
			Logger::Instance().Error(msg);
			throw std::out_of_range{msg};
		}
	}


	auto DataManager::UnregisterComponentFromEntity(const Entity* entity, const Component* component) -> void
	{
		if (const auto entityIt{FindEntityInternal(entity->Name())};
			entityIt != m_EntitiesAndComponents.end())
		{
			std::erase_if(entityIt->Components, [&](const auto& elem)
			{
				return elem.get() == component;
			});
		}
		else
		{
			const auto msg{"Entity at address [" + std::to_string(reinterpret_cast<unsigned long long>(entity)) + "] was not found while trying to unregister component at address [" + std::to_string(reinterpret_cast<unsigned long long>(component)) + "] from it."};
			Logger::Instance().Error(msg);
			throw std::out_of_range{msg};
		}
	}


	auto DataManager::ComponentsOfEntity(const Entity* entity) const -> const std::vector<std::unique_ptr<Component>>&
	{
		if (const auto it{FindEntityInternal(entity->Name())};
			it != m_EntitiesAndComponents.end())
		{
			return it->Components;
		}
		const auto msg{"Entity at address [" + std::to_string(reinterpret_cast<unsigned long long>(entity)) + "] was not found while trying access its components."};
		Logger::Instance().Error(msg);
		throw std::out_of_range{msg};
	}


	auto DataManager::SortEntities() -> void
	{
		std::ranges::sort(m_EntitiesAndComponents, [](const auto& left, const auto& right)
		{
			return left.Entity->Name() < right.Entity->Name();
		});
	}


	// SPOTLIGHTS

	auto DataManager::RegisterSpotLight(const SpotLight* const spotLight) -> void
	{
		m_SpotLights.push_back(spotLight);
	}


	auto DataManager::UnregisterSpotLight(const SpotLight* spotLight) -> void
	{
		std::erase(m_SpotLights, spotLight);
	}


	// POINTLIGHTS

	auto DataManager::RegisterPointLight(const PointLight* pointLight) -> void
	{
		m_PointLights.push_back(pointLight);
	}


	auto DataManager::UnregisterPointLight(const PointLight* pointLight) -> void
	{
		std::erase(m_PointLights, pointLight);
	}


	// TEXTURES

	auto DataManager::RegisterTexture(Texture* const texture) -> void
	{
		m_Textures.push_back(texture);
		SortTextures();
	}


	auto DataManager::UnregisterTexture(Texture* const texture) -> void
	{
		std::erase(m_Textures, texture);
		SortTextures();
	}


	auto DataManager::FindTexture(const std::filesystem::path& path) -> std::shared_ptr<Texture>
	{
		if (const auto it{
				std::ranges::lower_bound(m_Textures,
				                         path,
				                         [](const auto& elemPath, const auto& valPath)
				                         {
					                         return elemPath.compare(valPath);
				                         },
				                         [](const auto& texture) -> const auto&
				                         {
					                         return texture->Path;
				                         })
			};
			it != m_Textures.end() && (*it)->Path == path)
		{
			return (*it)->shared_from_this();
		}
		return nullptr;
	}


	auto DataManager::SortTextures() -> void
	{
		std::ranges::sort(m_Textures, [](const auto& left, const auto& right)
		{
			return left->Path.compare(right->Path);
		});
	}


	// SKYBOXES

	auto DataManager::CreateOrGetSkyboxImpl(std::filesystem::path allPaths) -> SkyboxImpl*
	{
		if (const auto it{m_Skyboxes.find(allPaths)};
			it != m_Skyboxes.end())
		{
			return &const_cast<SkyboxImpl&>(it->first);
		}
		return &const_cast<SkyboxImpl&>(m_Skyboxes.emplace(std::move(allPaths), std::vector<Skybox*>{}).first->first);
	}


	auto DataManager::DestroySkyboxImpl(const SkyboxImpl* const skybox) -> void
	{
		m_Skyboxes.erase(*skybox);
	}


	auto DataManager::RegisterSkyboxHandle(const SkyboxImpl* const skybox, Skybox* const handle) -> void
	{
		m_Skyboxes.at(*skybox).push_back(handle);
	}


	auto DataManager::UnregisterSkyboxHandle(const SkyboxImpl* const skybox, Skybox* const handle) -> void
	{
		std::erase(m_Skyboxes.at(*skybox), handle);
	}


	auto DataManager::SkyboxHandleCount(const SkyboxImpl* const skybox) const -> std::size_t
	{
		return m_Skyboxes.at(*skybox).size();
	}


	// MESHDATA

	auto DataManager::RegisterMeshDataGroup(MeshDataGroup* const meshData) -> void
	{
		m_MeshData.push_back(meshData);
		SortMeshData();
	}


	auto DataManager::UnregisterMeshDataGroup(MeshDataGroup* const meshData) -> void
	{
		std::erase(m_MeshData, meshData);
		SortMeshData();
	}


	auto DataManager::FindMeshDataGroup(const std::string& id) -> std::shared_ptr<MeshDataGroup>
	{
		if (const auto it{
				std::ranges::lower_bound(m_MeshData, id, [](const auto& elemId, const auto& value)
				                         {
					                         return elemId.compare(value) < 0;
				                         }, [](const auto& elem) -> const auto&
				                         {
					                         return elem->Id();
				                         })
			};
			it != m_MeshData.end() && (*it)->Id() == id)
		{
			return (*it)->shared_from_this();
		}
		return nullptr;
	}


	auto DataManager::SortMeshData() -> void
	{
		std::ranges::sort(m_MeshData, [](const auto& left, const auto& right)
		{
			return left->Id().compare(right->Id()) < 0;
		});
	}


	// MESHGROUPS

	auto DataManager::CreateOrGetMeshGroup(std::shared_ptr<const MeshDataGroup>&& meshDataGroup) -> const GlMeshGroup*
	{
		if (const auto it{m_Renderables.find(meshDataGroup)};
			it != m_Renderables.end())
		{
			return &it->first;
		}
		return &m_Renderables.emplace(std::move(meshDataGroup), decltype(m_Renderables)::mapped_type{}).first->first;
	}


	auto DataManager::RegisterInstanceForMeshGroup(const GlMeshGroup& meshGroup, RenderComponent* instance) -> void
	{
		m_Renderables.at(meshGroup).push_back(instance);
	}


	auto DataManager::UnregisterInstanceFromMeshGroup(const GlMeshGroup& meshGroup, RenderComponent* instance) -> void
	{
		const auto it{m_Renderables.find(meshGroup)};
		std::erase(it->second, instance);
		if (it->second.empty())
		{
			m_Renderables.erase(it);
		}
	}
}
