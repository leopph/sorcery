#include "Scene.hpp"


namespace leopph
{
	Scene::Scene(std::string name) :
		mName{ std::move(name) }
	{}


	void Scene::AddEntity(Entity* entity)
	{
		mEntities.emplace_back(entity);
	}


	void Scene::RemoveEntity(Entity* entity)
	{
		std::erase(mEntities, entity);
	}
}