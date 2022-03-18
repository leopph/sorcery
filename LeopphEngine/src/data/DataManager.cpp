#include "DataManager.hpp"

#include "../util/Logger.hpp"


namespace leopph::internal
{
	auto DataManager::Instance() -> DataManager&
	{
		static DataManager instance;
		return instance;
	}


	auto DataManager::Clear() -> void
	{
		// Since some Poelo destructors might invoke other Poelo deletions, its safer to destruct one by one, than to clear.
		while (m_Poelos.begin() != m_Poelos.end())
		{
			m_Poelos.erase(m_Poelos.begin());
		}
		// All containers should be empty at this point.
		Logger::Instance().Debug("DataManager cleared.");
	}


	auto DataManager::Store(std::unique_ptr<Poelo> poelo) -> void
	{
		m_Poelos.insert(std::move(poelo));
	}


	auto DataManager::Destroy(const Poelo* poelo) -> bool
	{
		if (const auto it{m_Poelos.find(poelo)}; it != m_Poelos.end())
		{
			m_Poelos.erase(it);
			return true;
		}
		return false;
	}


	auto DataManager::RegisterEntity(Entity* const entity) -> void
	{
		m_EntitiesAndComponents.emplace_back(entity);
		SortEntities();
	}


	auto DataManager::UnregisterEntity(const Entity* const entity) -> void
	{
		std::erase_if(m_EntitiesAndComponents, [&](const auto& elem)
		{
			return elem.Entity == entity;
		});
		SortEntities();
	}


	auto DataManager::FindEntity(const std::string& name) -> Entity*
	{
		const auto it = FindEntityInternal(name);
		return it != m_EntitiesAndComponents.end() ? it->Entity : nullptr;
	}


	auto DataManager::RegisterComponentForEntity(const Entity* const entity, Component* const component, const bool active) -> void
	{
		const auto it{FindEntityInternal(entity->Name())};
		auto& components{active ? it->ActiveComponents : it->InactiveComponents};
		components.push_back(component);
	}


	auto DataManager::UnregisterComponentFromEntity(const Entity* const entity, Component* const component, const bool active) -> void
	{
		const auto it{FindEntityInternal(entity->Name())};
		auto& components{active ? it->ActiveComponents : it->InactiveComponents};
		std::erase(components, component);
	}


	auto DataManager::ComponentsOfEntity(const Entity* const entity, const bool active) const -> std::span<Component* const>
	{
		const auto it{FindEntityInternal(entity->Name())};
		return active ? it->ActiveComponents : it->InactiveComponents;
	}


	auto DataManager::RegisterBehavior(Behavior* behavior, const bool active) -> void
	{
		(active ? m_ActiveBehaviors : m_InactiveBehaviors).push_back(behavior);
	}


	auto DataManager::UnregisterBehavior(const Behavior* behavior, const bool active) -> void
	{
		std::erase(active ? m_ActiveBehaviors : m_InactiveBehaviors, behavior);
	}


	auto DataManager::RegisterDirLight(const leopph::DirectionalLight* dirLight, const bool active) -> void
	{
		(active ? m_ActiveDirLights : m_InactiveDirLights).push_back(dirLight);
	}


	auto DataManager::UnregisterDirLight(const leopph::DirectionalLight* dirLight, const bool active) -> void
	{
		std::erase(active ? m_ActiveDirLights : m_InactiveDirLights, dirLight);
	}


	auto DataManager::DirectionalLight() const -> const leopph::DirectionalLight*
	{
		if (m_ActiveDirLights.empty())
		{
			return nullptr;
		}
		return m_ActiveDirLights.front();
	}


	auto DataManager::RegisterSpotLight(const SpotLight* spotLight, const bool active) -> void
	{
		(active ? m_ActiveSpotLights : m_InactiveSpotLights).push_back(spotLight);
	}


	auto DataManager::UnregisterSpotLight(const SpotLight* spotLight, const bool active) -> void
	{
		std::erase(active ? m_ActiveSpotLights : m_InactiveSpotLights, spotLight);
	}


	auto DataManager::RegisterPointLight(const PointLight* pointLight, const bool active) -> void
	{
		(active ? m_ActivePointLights : m_InactivePointLights).push_back(pointLight);
	}


	auto DataManager::UnregisterPointLight(const PointLight* pointLight, const bool active) -> void
	{
		std::erase(active ? m_ActivePointLights : m_InactivePointLights, pointLight);
	}


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
				                         }, [](const auto& elem)
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


	auto DataManager::RegisterGlMeshGroup(GlMeshGroup* glMeshGroup) -> void
	{
		m_Renderables.push_back(MeshGroupAndInstances{.MeshGroup = glMeshGroup, .ActiveInstances = {}, .InactiveInstances = {}});
	}


	auto DataManager::UnregisterGlMeshGroup(const GlMeshGroup* glMeshGroup) -> void
	{
		std::erase_if(m_Renderables, [glMeshGroup](const auto& elem)
		{
			return elem.MeshGroup == glMeshGroup;
		});
	}


	auto DataManager::FindGlMeshGroup(const MeshDataGroup* meshDataGroup) -> std::shared_ptr<GlMeshGroup>
	{
		if (const auto it = std::ranges::find_if(m_Renderables, [meshDataGroup](const auto& elem)
		{
			return elem.MeshGroup->MeshData().Id() == meshDataGroup->Id();
		});
			it != m_Renderables.end())
		{
			return it->MeshGroup->shared_from_this();
		}

		return nullptr;
	}


	auto DataManager::RegisterInstanceForMeshGroup(const GlMeshGroup* meshGroup, const RenderComponent* instance, const bool active) -> void
	{
		const auto it{FindMeshGroupInternal(meshGroup)};
		(active ? it->ActiveInstances : it->InactiveInstances).push_back(instance);
	}


	auto DataManager::UnregisterInstanceFromMeshGroup(const GlMeshGroup* meshGroup, const RenderComponent* instance, const bool active) -> void
	{
		const auto it{FindMeshGroupInternal(meshGroup)};
		std::erase(active ? it->ActiveInstances : it->InactiveInstances, instance);
	}


	auto DataManager::MeshGroupInstanceCount(const GlMeshGroup* const meshGroup) const -> std::size_t
	{
		const auto it{FindMeshGroupInternal(meshGroup)};
		return it->ActiveInstances.size() + it->InactiveInstances.size();
	}


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


	auto DataManager::SortEntities() -> void
	{
		std::ranges::sort(m_EntitiesAndComponents, [](const auto& left, const auto& right)
		{
			return left.Entity->Name() < right.Entity->Name();
		});
	}


	auto DataManager::SortMeshData() -> void
	{
		std::ranges::sort(m_MeshData, [](const auto* left, const auto* right)
		{
			return *left < *right;
		});
	}


	auto DataManager::SortTextures() -> void
	{
		std::ranges::sort(m_Textures, [](const auto& left, const auto& right)
		{
			return left->Path().compare(right->Path());
		});
	}


	auto DataManager::FindEntityInternal(const std::string& name) -> decltype(m_EntitiesAndComponents)::iterator
	{
		return FindEntityInternalCommon(this, name);
	}


	auto DataManager::FindEntityInternal(const std::string& name) const -> decltype(m_EntitiesAndComponents)::const_iterator
	{
		return FindEntityInternalCommon(this, name);
	}


	auto DataManager::FindMeshGroupInternal(const GlMeshGroup* const meshGroup) -> decltype(m_Renderables)::iterator
	{
		return FindMeshGroupInternalCommon(this, meshGroup);
	}


	auto DataManager::FindMeshGroupInternal(const GlMeshGroup* const meshGroup) const -> decltype(m_Renderables)::const_iterator
	{
		return FindMeshGroupInternalCommon(this, meshGroup);
	}
}
