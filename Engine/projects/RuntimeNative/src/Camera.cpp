#include "Camera.hpp"

namespace leopph
{
	Vector3 camPos{0, 0, 0};

	extern "C"
	{
		__declspec(dllexport) void set_cam_pos(Vector3 const* pos)
		{
			camPos = *pos;
		}
	}
}