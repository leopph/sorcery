#include "DataManager.hpp"

#include "Logger.hpp"

#include <algorithm>


namespace leopph::internal
{
	DataManager::~DataManager()
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


	auto DataManager::RegisterComponent(Entity const* const entity, ComponentPtr<> component, bool const active) -> void
	{
		auto const it{GetEntityIterator(entity->get_name())};
		(active ? it->ActiveComponents : it->InactiveComponents).push_back(std::move(component));
	}


	auto DataManager::UnregisterComponent(Entity const* const entity, Component const* component, bool const active) -> ComponentPtr<>
	{
		auto const entryIt = GetEntityIterator(entity->get_name());
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
		auto const it{GetEntityIterator(entity->get_name())};
		return active ? it->ActiveComponents : it->InactiveComponents;
	}


	auto DataManager::RegisterActiveBehavior(Behavior* behavior) -> void
	{
		m_ActiveBehaviors.push_back(behavior);
		SortActiveBehaviors();
	}


	auto DataManager::UnregisterActiveBehavior(Behavior const* behavior) -> void
	{
		std::erase(m_ActiveBehaviors, behavior);
	}


	auto DataManager::ActiveBehaviors() const noexcept -> std::span<Behavior* const>
	{
		return m_ActiveBehaviors;
	}


	auto DataManager::RegisterActiveDirLight(leopph::DirectionalLight const* dirLight) -> void
	{
		m_ActiveDirLights.push_back(dirLight);
	}


	auto DataManager::UnregisterActiveDirLight(leopph::DirectionalLight const* dirLight) -> void
	{
		std::erase(m_ActiveDirLights, dirLight);
	}


	auto DataManager::DirectionalLight() const -> leopph::DirectionalLight const*
	{
		return m_ActiveDirLights.empty() ? nullptr : m_ActiveDirLights.front();
	}


	auto DataManager::RegisterActiveSpotLight(SpotLight const* spotLight) -> void
	{
		m_ActiveSpotLights.push_back(spotLight);
	}


	auto DataManager::UnregisterActiveSpotLight(SpotLight const* spotLight) -> void
	{
		std::erase(m_ActiveSpotLights, spotLight);
	}


	auto DataManager::ActiveSpotLights() const noexcept -> std::span<SpotLight const* const>
	{
		return m_ActiveSpotLights;
	}


	auto DataManager::RegisterActivePointLight(PointLight const* pointLight) -> void
	{
		m_ActivePointLights.push_back(pointLight);
	}


	auto DataManager::UnregisterActivePointLight(PointLight const* pointLight) -> void
	{
		std::erase(m_ActivePointLights, pointLight);
	}


	auto DataManager::ActivePointLights() const noexcept -> std::span<PointLight const* const>
	{
		return m_ActivePointLights;
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
