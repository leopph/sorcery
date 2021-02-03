#include <entry.h>
#include <vector.h>
#include <matrix.h>
#include <iostream>

using leopph::Vector3;
using leopph::Matrix3;
using leopph::Matrix4;


void leopph::Init()
{
	Vector3 pos{ 0, 0, 0 };
	Vector3 forward{ 0, 0, -1 };
	Vector3 worldUp{ 0, 1, 0 };

	std::cout << Matrix4::LookAt(pos, forward, worldUp) << std::endl;
}