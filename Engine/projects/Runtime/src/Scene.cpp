#include "Scene.hpp"

#include "Node.hpp"


namespace leopph
{
	Scene::Scene(std::string name) :
		mName{std::move(name)}
	{ }



	std::span<Node* const> Scene::get_entities() const
	{
		return mEntities;
	}



	Scene::~Scene()
	{
		// Node destructor removes entity from this list
		while (!mEntities.empty())
		{
			delete mEntities.back();
		}
	}



	void Scene::add(Node* entity)
	{
		mEntities.push_back(entity);
	}



	void Scene::remove(Node* entity)
	{
		std::erase(mEntities, entity);
	}
}
