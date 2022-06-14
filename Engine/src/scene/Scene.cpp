#include "Scene.hpp"

#include "SceneManager.hpp"

#include <functional>
#include <memory>


namespace leopph
{
	auto Scene::CreateScene(const std::size_t id) -> Scene*
	{
		std::unique_ptr<Scene> scene{new Scene{id}};
		const auto ret{scene.get()};
		SceneManager::Instance().AddScene(std::move(scene));
		return ret;
	}


	auto Scene::CreateScene(const std::string_view name) -> Scene*
	{
		return CreateScene(NameToId(name));
	}


	auto Scene::NameToId(const std::string_view name) -> std::size_t
	{
		return std::hash<std::string_view>{}(name);
	}


	Scene::Scene(const std::size_t id) :
		m_Id{id}
	{}
}
