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
		Logger::get_instance().debug("DataManager cleared.");
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



	void DataManager::SortActiveBehaviors()
	{
		std::ranges::sort(m_ActiveBehaviors, BehaviorOrderFunc{}, [](auto const* behavior)
		{
			return behavior->get_update_index();
		});
	}
}
