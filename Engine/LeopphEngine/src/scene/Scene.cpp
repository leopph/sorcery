#include "Scene.hpp"

#include "SceneManager.hpp"

#include <functional>
#include <memory>


namespace leopph
{
	Scene* Scene::CreateScene(std::size_t const id)
	{
		std::unique_ptr<Scene> scene{new Scene{id}};
		auto const ret{scene.get()};
		SceneManager::Instance().AddScene(std::move(scene));
		return ret;
	}


	Scene* Scene::CreateScene(std::string_view const name)
	{
		return CreateScene(NameToId(name));
	}


	std::size_t Scene::NameToId(std::string_view const name)
	{
		return std::hash<std::string_view>{}(name);
	}


	Scene::Scene(std::size_t const id) :
		m_Id{id}
	{}
}
