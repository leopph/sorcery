#include <vector.h>
#include <matrix.h>
#include <entry.h>
#include <iostream>


void leopph::Init()
{
	leopph::Matrix4 m1{ 1, 2, 3, 4 };
	leopph::Matrix4 m2{ 1, 2, 3, 4 };
	auto res = m1 * m2;
	std::cout << res << std::endl;

	leopph::Vector3 v1{ 1, 3, -5 };
	leopph::Vector3 v2{ 4, -2, -1 };
	auto vres = Vector3::Dot(v1, v2);
	std::cout << vres << std::endl;
}