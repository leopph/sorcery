#include "Scene.hpp"


namespace leopph
{
	Scene::Scene(std::string name) :
		mName{std::move(name)}
	{ }



	std::span<Entity* const> Scene::get_entities() const
	{
		return mEntities;
	}



	void Scene::register_entity(Entity* entity)
	{
		mEntities.push_back(entity);
	}



	void Scene::unregister_entity(Entity* entity)
	{
		std::erase(mEntities, entity);
	}
}
