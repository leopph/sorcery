#include <array>
#include <cstring>
#include <vector>

#define DllExport __declspec(dllexport)

using Vec3f = std::array<float, 3>;

static std::vector<Vec3f> gPositions;
static std::array<unsigned char, 256> gKeyboardState{};
static float gLastFrameTimeSeconds{0};

unsigned char constexpr KEY_NEUTRAL = 0;
unsigned char constexpr KEY_DOWN = 1;
unsigned char constexpr KEY_HELD = 2;
unsigned char constexpr KEY_UP = 3;

extern "C"
{
	DllExport std::size_t add_position(float* vector)
	{
		gPositions.emplace_back();
		std::memcpy(gPositions.back().data(), vector, sizeof(Vec3f));
		return gPositions.size() - 1;
	}

	DllExport void update_position(std::size_t index, float* vector)
	{
		std::memcpy(gPositions[index].data(), vector, sizeof(Vec3f));
	}

	DllExport Vec3f const* get_positions()
	{
		return gPositions.data();
	}

	DllExport std::size_t get_num_positions()
	{
		return gPositions.size();
	}

	DllExport void update_keyboard_state(unsigned char const* const newState)
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

	DllExport bool get_key(unsigned char const key)
	{
		return gKeyboardState[key] == KEY_DOWN || gKeyboardState[key] == KEY_HELD;
	}

	DllExport bool get_key_down(unsigned char const key)
	{
		return gKeyboardState[key] == KEY_DOWN;
	}

	DllExport bool get_key_up(unsigned char const key)
	{
		return gKeyboardState[key] == KEY_UP;
	}

	DllExport float get_frame_time()
	{
		return gLastFrameTimeSeconds;
	}

	DllExport void set_frame_time(float seconds)
	{
		gLastFrameTimeSeconds = seconds;
	}
}