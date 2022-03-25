#include "DataManager.hpp"

#include "../util/Logger.hpp"

#include <algorithm>


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
		while (!m_Poelos.empty())
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


	auto DataManager::Destroy(Poelo const* poelo) -> bool
	{
		return std::erase_if(m_Poelos, [poelo](std::unique_ptr<Poelo> const& elem)
		{
			return elem.get() == poelo;
		});
	}


	auto DataManager::RegisterEntity(Entity* const entity) -> void
	{
		m_EntitiesAndComponents.emplace_back(entity);
		SortEntities();
	}


	auto DataManager::UnregisterEntity(Entity const* const entity) -> void
	{
		// Keeps the relative order, remains sorted.
		std::erase_if(m_EntitiesAndComponents, [entity](EntityEntry const& elem)
		{
			return *elem.Entity == *entity;
		});
	}


	auto DataManager::FindEntity(std::string const& name) -> Entity*
	{
		auto const it = GetEntityIterator(name);
		return it != m_EntitiesAndComponents.end() ? it->Entity : nullptr;
	}


	auto DataManager::RegisterComponentForEntity(Entity const* const entity, ComponentPtr<> component, bool const active) -> void
	{
		auto const it{GetEntityIterator(entity->Name())};
		(active ? it->ActiveComponents : it->InactiveComponents).push_back(std::move(component));
	}


	auto DataManager::UnregisterComponentFromEntity(Entity const* const entity, Component const* component, bool const active) -> ComponentPtr<>
	{
		auto const entryIt = GetEntityIterator(entity->Name());
		auto& components = active ? entryIt->ActiveComponents : entryIt->InactiveComponents; // no bounds checking here
		auto const compIt = std::ranges::find(components, component, [](auto const& elem) -> auto const*
		{
			return elem.get();
		});
		auto ret{std::move(*compIt)}; // no bounds checking here either
		components.erase(compIt); // no bounds checking
		return ret;
	}


	auto DataManager::ComponentsOfEntity(Entity const* const entity, bool const active) const -> std::span<ComponentPtr<> const>
	{
		auto const it{GetEntityIterator(entity->Name())};
		return active ? it->ActiveComponents : it->InactiveComponents;
	}


	auto DataManager::RegisterBehavior(Behavior* behavior, bool const active) -> void
	{
		(active ? m_ActiveBehaviors : m_InactiveBehaviors).push_back(behavior);

		if (active)
		{
			SortActiveBehaviors();
		}
	}


	auto DataManager::UnregisterBehavior(Behavior const* behavior, bool const active) -> void
	{
		std::erase(active ? m_ActiveBehaviors : m_InactiveBehaviors, behavior);
	}


	auto DataManager::RegisterDirLight(leopph::DirectionalLight const* dirLight, bool const active) -> void
	{
		(active ? m_ActiveDirLights : m_InactiveDirLights).push_back(dirLight);
	}


	auto DataManager::UnregisterDirLight(leopph::DirectionalLight const* dirLight, bool const active) -> void
	{
		std::erase(active ? m_ActiveDirLights : m_InactiveDirLights, dirLight);
	}


	auto DataManager::DirectionalLight() const -> leopph::DirectionalLight const*
	{
		return m_ActiveDirLights.empty() ? nullptr : m_ActiveDirLights.front();
	}


	auto DataManager::RegisterSpotLight(SpotLight const* spotLight, bool const active) -> void
	{
		(active ? m_ActiveSpotLights : m_InactiveSpotLights).push_back(spotLight);
	}


	auto DataManager::UnregisterSpotLight(SpotLight const* spotLight, bool const active) -> void
	{
		std::erase(active ? m_ActiveSpotLights : m_InactiveSpotLights, spotLight);
	}


	auto DataManager::RegisterPointLight(PointLight const* pointLight, bool const active) -> void
	{
		(active ? m_ActivePointLights : m_InactivePointLights).push_back(pointLight);
	}


	auto DataManager::UnregisterPointLight(PointLight const* pointLight, bool const active) -> void
	{
		std::erase(active ? m_ActivePointLights : m_InactivePointLights, pointLight);
	}


	auto DataManager::RegisterMeshGroup(std::shared_ptr<MeshGroup const> const& meshGroup) -> void
	{
		m_MeshGroups.insert_or_assign(meshGroup->Id, meshGroup);
	}


	auto DataManager::FindMeshGroup(std::string const& id) -> std::shared_ptr<MeshGroup const>
	{
		if (auto const it = m_MeshGroups.find(id); it != m_MeshGroups.end())
		{
			if (auto ret = it->second.lock())
			{
				return ret;
			}

			m_MeshGroups.erase(it);
		}

		return nullptr;
	}


	auto DataManager::RegisterGlMeshGroup(std::shared_ptr<GlMeshGroup> const& glMeshGroup) -> void
	{
		m_Renderables.insert_or_assign(glMeshGroup->MeshGroup()->Id, RenderableAndInstances{.MeshGroup = glMeshGroup, .ActiveInstances = {}, .InactiveInstances = {}});
	}


	auto DataManager::FindGlMeshGroup(std::string const& meshGroupId) -> std::shared_ptr<GlMeshGroup>
	{
		if (auto const it = m_Renderables.find(meshGroupId); it != m_Renderables.end())
		{
			if (auto ret = it->second.MeshGroup.lock())
			{
				return ret;
			}

			m_Renderables.erase(it);
		}

		return nullptr;
	}


	auto DataManager::RegisterInstanceForGlMeshGroup(std::string const& meshGroupId, RenderComponent const* instance, bool const active) -> void
	{
		auto& [glMeshGroup, activeInstances, inactiveInstances] = m_Renderables.at(meshGroupId);
		(active ? activeInstances : inactiveInstances).push_back(instance);
	}


	auto DataManager::UnregisterInstanceFromGlMeshGroup(std::string const& meshGroupId, RenderComponent const* instance, bool const active) -> void
	{
		auto& [glMeshGroup, activeInstances, inactiveInstances] = m_Renderables.at(meshGroupId);
		std::erase(active ? activeInstances : inactiveInstances, instance);
	}


	auto DataManager::GlMeshGroupInstanceCount(std::string const& meshId) const -> std::size_t
	{
		auto& [glMeshGroup, activeInstances, inactiveInstances] = m_Renderables.at(meshId);
		return activeInstances.size() + inactiveInstances.size();
	}


	auto DataManager::CreateOrGetSkyboxImpl(std::filesystem::path allPaths) -> SkyboxImpl*
	{
		if (auto const it{m_Skyboxes.find(allPaths)};
			it != m_Skyboxes.end())
		{
			return &const_cast<SkyboxImpl&>(it->first);
		}
		return &const_cast<SkyboxImpl&>(m_Skyboxes.emplace(std::move(allPaths), std::vector<Skybox*>{}).first->first);
	}


	auto DataManager::DestroySkyboxImpl(SkyboxImpl const* const skybox) -> void
	{
		m_Skyboxes.erase(*skybox);
	}


	auto DataManager::RegisterSkyboxHandle(SkyboxImpl const* const skybox, Skybox* const handle) -> void
	{
		m_Skyboxes.at(*skybox).push_back(handle);
	}


	auto DataManager::UnregisterSkyboxHandle(SkyboxImpl const* const skybox, Skybox* const handle) -> void
	{
		std::erase(m_Skyboxes.at(*skybox), handle);
	}


	auto DataManager::SkyboxHandleCount(SkyboxImpl const* const skybox) const -> std::size_t
	{
		return m_Skyboxes.at(*skybox).size();
	}


	auto DataManager::SortEntities() -> void
	{
		std::ranges::sort(m_EntitiesAndComponents, EntityOrderFunc{}, [](auto const& elem) -> auto const&
		{
			return *elem.Entity;
		});
	}


	auto DataManager::SortActiveBehaviors() -> void
	{
		std::ranges::sort(m_ActiveBehaviors, BehaviorOrderFunc{}, [](auto const* behavior)
		{
			return behavior->UpdateIndex();
		});
	}


	auto DataManager::GetEntityIterator(std::string const& name) -> decltype(m_EntitiesAndComponents)::iterator
	{
		return GetEntityIteratorCommon(this, name);
	}


	auto DataManager::GetEntityIterator(std::string const& name) const -> decltype(m_EntitiesAndComponents)::const_iterator
	{
		return GetEntityIteratorCommon(this, name);
	}
}
