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


	void DataManager::Store(std::unique_ptr<Poelo> poelo)
	{
		m_Poelos.insert(std::move(poelo));
	}


	bool DataManager::Destroy(Poelo const* poelo)
	{
		return std::erase_if(m_Poelos, [poelo](std::unique_ptr<Poelo> const& elem)
		{
			return elem.get() == poelo;
		});
	}


	void DataManager::RegisterEntity(Entity* const entity)
	{
		m_EntitiesAndComponents.emplace_back(entity);
		SortEntities();
	}


	void DataManager::UnregisterEntity(Entity const* const entity)
	{
		// Keeps the relative order, remains sorted.
		std::erase_if(m_EntitiesAndComponents, [entity](EntityEntry const& elem)
		{
			return *elem.Entity == *entity;
		});
	}


	Entity* DataManager::FindEntity(std::string const& name)
	{
		auto const it = GetEntityIterator(name);
		return it != m_EntitiesAndComponents.end() ? it->Entity : nullptr;
	}


	void DataManager::RegisterComponent(Entity const* const entity, ComponentPtr<> component, bool const active)
	{
		auto const it{GetEntityIterator(entity->get_name())};
		(active ? it->ActiveComponents : it->InactiveComponents).push_back(std::move(component));
	}


	ComponentPtr<> DataManager::UnregisterComponent(Entity const* const entity, Component const* component, bool const active)
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


	std::span<ComponentPtr<> const> DataManager::ComponentsOfEntity(Entity const* const entity, bool const active) const
	{
		auto const it{GetEntityIterator(entity->get_name())};
		return active ? it->ActiveComponents : it->InactiveComponents;
	}


	void DataManager::RegisterActiveBehavior(Behavior* behavior)
	{
		m_ActiveBehaviors.push_back(behavior);
		SortActiveBehaviors();
	}


	void DataManager::UnregisterActiveBehavior(Behavior const* behavior)
	{
		std::erase(m_ActiveBehaviors, behavior);
	}


	std::span<Behavior* const> DataManager::ActiveBehaviors() const noexcept
	{
		return m_ActiveBehaviors;
	}


	void DataManager::RegisterActiveDirLight(leopph::DirectionalLight const* dirLight)
	{
		m_ActiveDirLights.push_back(dirLight);
	}


	void DataManager::UnregisterActiveDirLight(leopph::DirectionalLight const* dirLight)
	{
		std::erase(m_ActiveDirLights, dirLight);
	}


	leopph::DirectionalLight const* DataManager::DirectionalLight() const
	{
		return m_ActiveDirLights.empty() ? nullptr : m_ActiveDirLights.front();
	}


	void DataManager::RegisterActiveSpotLight(SpotLight const* spotLight)
	{
		m_ActiveSpotLights.push_back(spotLight);
	}


	void DataManager::UnregisterActiveSpotLight(SpotLight const* spotLight)
	{
		std::erase(m_ActiveSpotLights, spotLight);
	}


	std::span<SpotLight const* const> DataManager::ActiveSpotLights() const noexcept
	{
		return m_ActiveSpotLights;
	}


	void DataManager::RegisterActivePointLight(PointLight const* pointLight)
	{
		m_ActivePointLights.push_back(pointLight);
	}


	void DataManager::UnregisterActivePointLight(PointLight const* pointLight)
	{
		std::erase(m_ActivePointLights, pointLight);
	}


	std::span<PointLight const* const> DataManager::ActivePointLights() const noexcept
	{
		return m_ActivePointLights;
	}


	void DataManager::SortEntities()
	{
		std::ranges::sort(m_EntitiesAndComponents, EntityOrderFunc{}, [](auto const& elem) -> auto const&
		{
			return *elem.Entity;
		});
	}


	void DataManager::SortActiveBehaviors()
	{
		std::ranges::sort(m_ActiveBehaviors, BehaviorOrderFunc{}, [](auto const* behavior)
		{
			return behavior->UpdateIndex();
		});
	}


	decltype(DataManager::m_EntitiesAndComponents)::iterator DataManager::GetEntityIterator(std::string const& name)
	{
		return GetEntityIteratorCommon(this, name);
	}


	decltype(DataManager::m_EntitiesAndComponents)::const_iterator DataManager::GetEntityIterator(std::string const& name) const
	{
		return GetEntityIteratorCommon(this, name);
	}
}
