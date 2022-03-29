#include "SceneSwitcher.hpp"

#include <algorithm>
#include <queue>


namespace demo
{
	auto SceneSwitcher::Scene::Add(leopph::Entity* entity) -> void
	{
		m_Entities.push_back(entity);
	}


	auto SceneSwitcher::Scene::Activate() const -> void
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

			for (auto const* child : entity->Transform()->Children())
			{
				q.push(child->Owner());
			}

			entity->ActivateAllComponents();
		}

		if (m_ActivationCallback)
		{
			m_ActivationCallback();
		}
	}


	auto SceneSwitcher::Scene::Deactivate() const -> void
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

			for (auto const* child : entity->Transform()->Children())
			{
				q.push(child->Owner());
			}

			entity->DeactiveAllComponents();
		}
	}


	auto SceneSwitcher::Scene::SetActivationCallback(std::function<void()> callback) -> void
	{
		m_ActivationCallback = std::move(callback);
	}


	auto SceneSwitcher::OnFrameUpdate() -> void
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


	auto SceneSwitcher::CreateOrGetScene(leopph::KeyCode const key) -> Scene&
	{
		return m_Scenes[key];
	}
}
