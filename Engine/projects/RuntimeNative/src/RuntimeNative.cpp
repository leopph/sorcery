#include "RuntimeNative.hpp"

#include <unordered_map>
#include <memory>


namespace
{
	std::vector<Vector3> gPositions;
	Vector3 gCamPos{0, 0, 0};

	std::vector<PointLight*> gPointLights;

	std::unordered_map<std::size_t, std::unique_ptr<Node>> gNodes;

	std::size_t generate_next_node_id()
	{
		static std::size_t currentId{0};
		return currentId++;
	}
}


extern "C"
{
	API std::size_t new_node()
	{
		std::size_t const id{generate_next_node_id()};
		//gNodes.emplace(id, std::make_unique<Node>(Vector3{0, 0, 0}));
		return id;
	}

	API void delete_node(std::size_t const id)
	{
		//gNodes.erase(id);
	}


	std::size_t add_position(Vector3 const* vector)
	{
		gPositions.emplace_back(*vector);
		return gPositions.size() - 1;
	}


	void update_position(std::size_t const index, Vector3 const* vector)
	{
		gPositions[index] = *vector;
	}


	std::vector<Vector3> const* get_positions()
	{
		return &gPositions;
	}


	Vector3 const* get_cam_pos()
	{
		return &gCamPos;
	}


	void set_cam_pos(Vector3 const* pos)
	{
		gCamPos = *pos;
	}


	void register_point_light(PointLight* const light)
	{
		gPointLights.emplace_back(light);
	}


	std::vector<PointLight*>* get_point_lights()
	{
		return &gPointLights;
	}
}
