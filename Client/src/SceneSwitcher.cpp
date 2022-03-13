#include "SceneSwitcher.hpp"

#include <algorithm>
#include <utility>


namespace demo
{
	auto SceneSwitcher::Scene::Add(leopph::Entity* const entity) -> void
	{
		m_Pointer->second.push_back(entity);
	}


	auto SceneSwitcher::Scene::Id() const -> IdType
	{
		return m_Pointer->first;
	}


	SceneSwitcher::Scene::Scene(PointerType pointer) :
		m_Pointer{pointer}
	{}


	auto SceneSwitcher::CreateScene() -> Scene
	{
		return Scene{&*m_Scenes.emplace(GenerateId(), SceneDataType{}).first};
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


	auto SceneSwitcher::ActivateScene(const Scene scene) -> void
	{
		ActivateScene(scene.Id());
	}


	auto SceneSwitcher::ActiveScene() -> std::optional<SceneSwitcher::Scene>
	{
		if (!m_Active)
		{
			return {};
		}

		return Scene{&*m_Scenes.find(*m_Active)};
	}


	auto SceneSwitcher::GenerateId() noexcept -> SceneSwitcher::IdType
	{
		static IdType nextId{0};
		return nextId++;
	}
}
