#include <Entity.hpp>

#include <yaml-cpp/yaml.h>

void Serialize(std::span<leopph::Entity const* const> entities, std::ostream& os)
{
	YAML::Emitter emitter{ os };

	for (auto const& entity : entities)
	{
		YAML::Node node;

		node["name"] = entity->get_name().data();

		for (int i = 0; i < 3; i++)
		{
			node["position"].push_back(entity->get_position()[i]);
		}

		node["rotation"].push_back(entity->get_rotation().w);
		node["rotation"].push_back(entity->get_rotation().x);
		node["rotation"].push_back(entity->get_rotation().y);
		node["rotation"].push_back(entity->get_rotation().z);

		for (int i = 0; i < 3; i++)
		{
			node["scale"].push_back(entity->get_scale()[i]);
		}

		emitter << node;
	}
}