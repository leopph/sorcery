#include "SceneSwitcher.hpp"

#include <algorithm>
#include <queue>


namespace demo
{
	void SceneSwitcher::Scene::Add(leopph::Entity* entity)
	{
		m_Entities.push_back(entity);
	}


	void SceneSwitcher::Scene::Activate() const
	{
		std::queue<leopph::Entity*> q;

		std::ranges::for_each(m_Entities, [&q](auto* entity)
		{
			q.push(entity);
		});

		while (!q.empty())
		{
			auto const* entity = q.front();
			q.pop();

			for (auto const* child : entity->get_transform().get_children())
			{
				q.push(child->get_entity());
			}

			entity->activate_all_components();
		}

		if (m_ActivationCallback)
		{
			m_ActivationCallback();
		}
	}


	void SceneSwitcher::Scene::Deactivate() const
	{
		std::queue<leopph::Entity*> q;

		std::ranges::for_each(m_Entities, [&q](auto* entity)
		{
			q.push(entity);
		});

		while (!q.empty())
		{
			auto const* entity = q.front();
			q.pop();

			for (auto const* child : entity->get_transform().get_children())
			{
				q.push(child->get_entity());
			}

			entity->deactive_all_components();
		}
	}


	void SceneSwitcher::Scene::SetActivationCallback(std::function<void()> callback)
	{
		m_ActivationCallback = std::move(callback);
	}


	void SceneSwitcher::OnFrameUpdate()
	{
		std::ranges::for_each(m_Scenes, [this](auto& pair)
		{
			if (leopph::Input::GetKeyDown(pair.first))
			{
				std::ranges::for_each(m_Scenes, [](auto& pair2)
				{
					pair2.second.Deactivate();
				});

				pair.second.Activate();
			}
		});
	}


	SceneSwitcher::Scene& SceneSwitcher::CreateOrGetScene(leopph::KeyCode const key)
	{
		return m_Scenes[key];
	}
}
