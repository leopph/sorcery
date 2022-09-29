#include <array>
#include <cstring>
#include <vector>

#define DllExport __declspec(dllexport)

using Vec3f = std::array<float, 3>;

static std::vector<Vec3f> gPositions;

extern "C"
{
	DllExport void add_position(float* vector)
	{
		gPositions.emplace_back();
		std::memcpy(gPositions.back().data(), vector, sizeof(Vec3f));
	}

	DllExport Vec3f const* get_positions()
	{
		return gPositions.data();
	}

	DllExport std::size_t get_num_positions()
	{
		return gPositions.size();
	}
}