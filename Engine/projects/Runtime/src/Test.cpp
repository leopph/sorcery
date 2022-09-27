#include <cstring>


namespace leopph
{
	__declspec(dllexport) float* gData = new float[2]{};


	extern "C"
	{
		__declspec(dllexport) void take_vec2(float const* vec)
		{
			std::memcpy(gData, vec, 2 * sizeof(float));
		}
	}
}
