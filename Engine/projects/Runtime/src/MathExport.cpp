#define DllExport __declspec(dllexport)

#include "Math.hpp"


extern "C"
{
	namespace leopph
	{
		DllExport Vector2* vec2_new(float const x, float const y)
		{
			return new Vector2{x, y};
		}



		DllExport void vec2_del(Vector2 const* const vector)
		{
			delete vector;
		}



		DllExport float vec2_lngth(Vector2 const* const vector)
		{
			return vector->length();
		}



		DllExport Vector2* vec2_normd(Vector2 const* const vector)
		{
			return new Vector2{vector->normalized()};
		}



		DllExport void vec2_norm(Vector2* const vector)
		{
			vector->normalize();
		}



		DllExport float vec2_get_elem(Vector2* const vector, int const index)
		{
			return vector->operator[](index);
		}



		DllExport void vec2_set_elem(Vector2* vector, int const index, float const value)
		{
			vector->operator[](index) = value;
		}

		DllExport void mulby2(int* arr, int const size)
		{
			for (int i = 0; i < size; i++)
			{
				arr[i] *= 2;
			}
		}
	}
}
