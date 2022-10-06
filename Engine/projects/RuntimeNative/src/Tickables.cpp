#include "RuntimeNative.hpp"

#include <unordered_map>
#include <memory>


namespace leopph
{
	namespace
	{
		std::vector<Vector3> gPositions;
		Vector3 gCamPos{0, 0, 0};

		std::vector<PointLight*> gPointLights;
	}


	extern "C"
	{
		
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
}