#include <Entity.hpp>
#include <Components.hpp>

#include <yaml-cpp/yaml.h>

void Serialize(std::span<std::unique_ptr<leopph::Entity> const> entities, std::ostream& os)
{
	YAML::Emitter emitter{os};

	for (auto const& entity : entities)
	{
		YAML::Node node;

		node["name"] = entity->name.data();

		for (int i = 0; i < 3; i++)
		{
			node["position"].push_back(entity->transform->GetLocalPosition()[i]);
		}

		node["rotation"].push_back(entity->transform->GetLocalRotation().w);
		node["rotation"].push_back(entity->transform->GetLocalRotation().x);
		node["rotation"].push_back(entity->transform->GetLocalRotation().y);
		node["rotation"].push_back(entity->transform->GetLocalRotation().z);

		for (int i = 0; i < 3; i++)
		{
			node["scale"].push_back(entity->transform->GetLocalScale()[i]);
		}

		emitter << node;
	}
}