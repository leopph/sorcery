#include "Scene.hpp"


namespace leopph
{
	Scene::Scene(std::string mName) :
		mName{ std::move(mName) }
	{}


	Entity* Scene::CreateEntity() {
		auto const entity{ new Entity{} };
		entity->SetScene(this);
		return mEntities.emplace_back(entity).get();
	}


	void Scene::DestroyEntity(Entity const* const entity) {
		if (entity) {
			std::erase_if(mEntities, [entity](auto const& ownedEntity) {
				return ownedEntity->GetGuid() == entity->GetGuid();
			});
		}
	}

	auto Scene::GetSerializationType() const -> Type {
		return Type::Scene;
	}
}
