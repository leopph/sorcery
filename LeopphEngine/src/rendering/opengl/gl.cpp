#include "gl.h"

#include <glad/glad.h>

#include <iostream>

namespace leopph::impl
{
	bool InitGL()
	{
		return gladLoadGL();
	}
}