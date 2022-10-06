#pragma once

#include "Math.hpp"

#include <cstddef>
#include <vector>


namespace leopph
{
	struct Object
	{
		std::size_t id;
	};


	struct Node : Object
	{
		Vector3 position;
	};


	struct Color
	{
		unsigned char r, g, b, a;
	};


	struct PointLight
	{
		Color color;
		Vector3 pos;
	};


	extern "C"
	{
		LEOPPHAPI std::size_t add_position(Vector3 const* vector);
		LEOPPHAPI void update_position(std::size_t index, Vector3 const* vector);
		LEOPPHAPI std::vector<Vector3> const* get_positions();

		LEOPPHAPI Vector3 const* get_cam_pos();
		LEOPPHAPI void set_cam_pos(Vector3 const* pos);

		LEOPPHAPI void update_keyboard_state();
		LEOPPHAPI bool get_key(unsigned char const key);
		LEOPPHAPI bool get_key_down(unsigned char const key);
		LEOPPHAPI bool get_key_up(unsigned char const key);

		LEOPPHAPI float get_full_time();
		LEOPPHAPI float get_frame_time();
		LEOPPHAPI void init_time();
		LEOPPHAPI void measure_time();

		LEOPPHAPI void register_point_light(PointLight* const light);
		LEOPPHAPI std::vector<PointLight*>* get_point_lights();
	}
}