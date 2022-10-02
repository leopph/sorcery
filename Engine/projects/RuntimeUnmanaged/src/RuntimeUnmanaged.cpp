#include "RuntimeUnmanaged.hpp"

#include <cstring>


unsigned char constexpr KEY_NEUTRAL = 0;
unsigned char constexpr KEY_DOWN = 1;
unsigned char constexpr KEY_HELD = 2;
unsigned char constexpr KEY_UP = 3;


namespace
{
	std::vector<Vector3> gPositions;
	Vector3 gCamPos{0, 0, 0};

	std::array<unsigned char, 256> gKeyboardState{};

	float gLastFrameTimeSeconds{0};

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


	void update_keyboard_state(unsigned char const* const newState)
	{
		for (int i = 0; i < 256; i++)
		{
			if (newState[i] & 0x80)
			{
				if (gKeyboardState[i] == KEY_DOWN)
				{
					gKeyboardState[i] = KEY_HELD;
				}
				else
				{
					gKeyboardState[i] = KEY_DOWN;
				}
			}
			else
			{
				if (gKeyboardState[i] == KEY_UP)
				{
					gKeyboardState[i] = KEY_NEUTRAL;
				}
				else
				{
					gKeyboardState[i] = KEY_UP;
				}
			}
		}
	}


	bool get_key(unsigned char const key)
	{
		return gKeyboardState[key] == KEY_DOWN || gKeyboardState[key] == KEY_HELD;
	}


	bool get_key_down(unsigned char const key)
	{
		return gKeyboardState[key] == KEY_DOWN;
	}


	bool get_key_up(unsigned char const key)
	{
		return gKeyboardState[key] == KEY_UP;
	}


	float get_frame_time()
	{
		return gLastFrameTimeSeconds;
	}

	void set_frame_time(float seconds)
	{
		gLastFrameTimeSeconds = seconds;
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