#include <entry.h>
#include <vector.h>
#include <matrix.h>
#include <iostream>
#include <leopphmath.h>

using leopph::Vector3;
using leopph::Matrix3;
using leopph::Matrix4;


void leopph::Init()
{
	std::cout << Matrix4::Perspective(57, 1.67f, 1, 100) << std::endl;
}