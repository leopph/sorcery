#include "SceneSwitcher.hpp"

#include <algorithm>
#include <utility>


namespace demo
{
	auto SceneSwitcher::RegisterScene(std::vector<leopph::Entity*> entities) -> std::size_t
	{
		std::size_t id;
		do
		{
			id = GenerateId();
		}
		while (m_Scenes.contains(id));
		m_Scenes[id] = std::move(entities);
		return id;
	}


	auto SceneSwitcher::ActivateScene(const std::size_t id) -> void
	{
		decltype(m_Scenes)::iterator it;

		if (it = m_Scenes.find(id); it == m_Scenes.end())
		{
			return;
		}

		if (m_Active)
		{
			std::ranges::for_each(m_Scenes[*m_Active], [](const auto ent)
			{
				ent->DeactiveAllComponents();
			});
		}

		m_Active = id;
		std::ranges::for_each(it->second, [](const auto ent)
		{
			ent->ActivateAllComponents();
		});
	}


	auto SceneSwitcher::GenerateId() noexcept -> std::size_t
	{
		static std::size_t nextId{0};
		return nextId++;
	}
}
