#include "DataManager.hpp"

#include "../util/logger.h"


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
		Logger::Instance().Debug("DataManager cleared.");
	}


	// BEHAVIORS

	auto DataManager::RegisterActiveBehavior(Behavior* behavior) -> void
	{
		m_ActiveBehaviors.push_back(behavior);
	}


	auto DataManager::RegisterInactiveBehavior(Behavior* behavior) -> void
	{
		m_InactiveBehaviors.push_back(behavior);
	}


	auto DataManager::UnregisterActiveBehavior(const Behavior* behavior) -> void
	{
		std::erase(m_ActiveBehaviors, behavior);
	}


	auto DataManager::UnregisterInactiveBehavior(const Behavior* behavior) -> void
	{
		std::erase(m_InactiveBehaviors, behavior);
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


	auto DataManager::RegisterActiveComponentForEntity(std::unique_ptr<Component>&& component) -> void
	{
		FindEntityInternal(component->Entity()->Name())->ActiveComponents.emplace_back(std::move(component));
	}


	auto DataManager::RegisterInactiveComponentForEntity(std::unique_ptr<Component>&& component) -> void
	{
		FindEntityInternal(component->Entity()->Name())->InactiveComponents.emplace_back(std::move(component));
	}


	auto DataManager::UnregisterActiveComponentFromEntity(const Component* component) -> std::unique_ptr<Component>
	{
		return EraseComponentInternal(FindEntityInternal(component->Entity()->Name())->ActiveComponents, component);
	}


	auto DataManager::UnregisterInactiveComponentFromEntity(const Component* component) -> std::unique_ptr<Component>
	{
		return EraseComponentInternal(FindEntityInternal(component->Entity()->Name())->InactiveComponents, component);
	}


	auto DataManager::ComponentsOfEntity(const Entity* entity) const -> const std::vector<std::unique_ptr<Component>>&
	{
		return FindEntityInternal(entity->Name())->ActiveComponents;
	}


	auto DataManager::SortEntities() -> void
	{
		std::ranges::sort(m_EntitiesAndComponents, [](const auto& left, const auto& right)
		{
			return left.Entity->Name() < right.Entity->Name();
		});
	}


	// SPOTLIGHTS

	auto DataManager::RegisterActiveSpotLight(const SpotLight* const spotLight) -> void
	{
		m_ActiveSpotLights.push_back(spotLight);
	}


	auto DataManager::RegisterInactiveSpotLight(const SpotLight* spotLight) -> void
	{
		m_InactiveSpotLights.push_back(spotLight);
	}


	auto DataManager::UnregisterActiveSpotLight(const SpotLight* spotLight) -> void
	{
		std::erase(m_ActiveSpotLights, spotLight);
	}


	auto DataManager::UnregisterInactiveSpotLight(const SpotLight* spotLight) -> void
	{
		std::erase(m_InactiveSpotLights, spotLight);
	}


	// POINTLIGHTS

	auto DataManager::RegisterActivePointLight(const PointLight* pointLight) -> void
	{
		m_ActivePointLights.push_back(pointLight);
	}


	auto DataManager::RegisterInactivePointLight(const PointLight* pointLight) -> void
	{
		m_InactivePointLights.push_back(pointLight);
	}


	auto DataManager::UnregisterActivePointLight(const PointLight* pointLight) -> void
	{
		std::erase(m_ActivePointLights, pointLight);
	}


	auto DataManager::UnregisterInactivePointLight(const PointLight* pointLight) -> void
	{
		std::erase(m_InactivePointLights, pointLight);
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
					                         return texture->Path();
				                         })
			};
			it != m_Textures.end() && (*it)->Path() == path)
		{
			return (*it)->shared_from_this();
		}
		return nullptr;
	}


	auto DataManager::SortTextures() -> void
	{
		std::ranges::sort(m_Textures, [](const auto& left, const auto& right)
		{
			return left->Path().compare(right->Path());
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
		if (const auto it{
			std::ranges::find_if(m_Renderables, [&](const auto& elem)
			{
				return elem.MeshGroup->MeshData() == *meshDataGroup;
			})
		}; it != m_Renderables.end())
		{
			return it->MeshGroup.get();
		}
		return m_Renderables.emplace_back(std::make_unique<GlMeshGroup>(std::move(meshDataGroup))).MeshGroup.get();
	}


	auto DataManager::DestroyMeshGroup(const GlMeshGroup* const meshGroup) -> void
	{
		m_Renderables.erase(FindMeshGroupInternal(meshGroup));
	}


	auto DataManager::RegisterActiveInstanceForMeshGroup(const GlMeshGroup* const meshGroup, const RenderComponent* const instance) -> void
	{
		FindMeshGroupInternal(meshGroup)->ActiveInstances.push_back(instance);
	}


	auto DataManager::RegisterInactiveInstanceForMeshGroup(const GlMeshGroup* meshGroup, const RenderComponent* instance) -> void
	{
		FindMeshGroupInternal(meshGroup)->InactiveInstances.push_back(instance);
	}


	auto DataManager::UnregisterActiveInstanceFromMeshGroup(const GlMeshGroup* const meshGroup, const RenderComponent* const instance) -> void
	{
		std::erase(FindMeshGroupInternal(meshGroup)->ActiveInstances, instance);
	}


	auto DataManager::UnregisterInactiveInstanceFromMeshGroup(const GlMeshGroup* meshGroup, const RenderComponent* instance) -> void
	{
		std::erase(FindMeshGroupInternal(meshGroup)->InactiveInstances, instance);
	}


	auto DataManager::UnregisterActiveDirLight(const leopph::DirectionalLight* dirLight) -> void
	{
		std::erase(m_ActiveDirLights, dirLight);
	}


	auto DataManager::RegisterInactiveDirLight(const leopph::DirectionalLight* dirLight) -> void
	{
		m_InactiveDirLights.push_back(dirLight);
	}


	auto DataManager::RegisterActiveDirLight(const leopph::DirectionalLight* dirLight) -> void
	{
		m_ActiveDirLights.push_back(dirLight);
	}


	auto DataManager::UnregisterInactiveDirLight(const leopph::DirectionalLight* dirLight) -> void
	{
		std::erase(m_InactiveDirLights, dirLight);
	}


	auto DataManager::DirectionalLight() const -> const leopph::DirectionalLight*
	{
		if (m_ActiveDirLights.empty())
		{
			return nullptr;
		}
		return m_ActiveDirLights.front();
	}


	auto DataManager::MeshGroupInstanceCount(const GlMeshGroup* const meshGroup) const -> std::size_t
	{
		const auto it{FindMeshGroupInternal(meshGroup)};
		return it->ActiveInstances.size() + it->InactiveInstances.size();
	}


	auto DataManager::FindMeshGroupInternal(const GlMeshGroup* const meshGroup) -> decltype(m_Renderables)::iterator
	{
		return FindMeshGroupInternalCommon(this, meshGroup);
	}


	auto DataManager::FindMeshGroupInternal(const GlMeshGroup* const meshGroup) const -> decltype(m_Renderables)::const_iterator
	{
		return FindMeshGroupInternalCommon(this, meshGroup);
	}


	auto DataManager::EraseComponentInternal(std::vector<std::unique_ptr<Component>>& components, const Component* component) -> std::unique_ptr<Component>
	{
		for (auto it = components.begin(); it != components.end(); ++it)
		{
			if (it->get() == component)
			{
				auto ret{std::move(*it)};
				components.erase(it);
				return ret;
			}
		}
		return nullptr;
	}
}
