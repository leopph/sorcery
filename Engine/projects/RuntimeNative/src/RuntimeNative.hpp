#pragma once

#include <cstddef>
#include <vector>


#ifdef LEOPPH_NATIVE_RUNTIME_EXPORT
#define API __declspec(dllexport)
#else
#define API __declspec(dllimport)
#endif


struct Vector2
{
	float x, y;
};


struct Vector3
{
	float x, y, z;
};


struct Vector4
{
	float x, y, z, w;
};


struct Quaternion
{
	float w, x, y, z;
};


struct Matrix3
{
	float data[9];
};


struct Matrix4
{
	float data[16];
};


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
	API std::size_t new_node();
	API void delete_node(std::size_t id);

	API std::size_t add_position(Vector3 const* vector);
	API void update_position(std::size_t index, Vector3 const* vector);
	API std::vector<Vector3> const* get_positions();

	API Vector3 const* get_cam_pos();
	API void set_cam_pos(Vector3 const* pos);

	API void update_keyboard_state();
	API bool get_key(unsigned char const key);
	API bool get_key_down(unsigned char const key);
	API bool get_key_up(unsigned char const key);

	API float get_full_time();
	API float get_frame_time();
	API void init_time();
	API void measure_time();

	API void register_point_light(PointLight* const light);
	API std::vector<PointLight*>* get_point_lights();
}