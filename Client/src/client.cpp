#include <vector.h>
#include <entry.h>
#include <iostream>


void leopph::Init()
{
	//std::cout << leopph::Matrix4{ 1, 2, 3, 4 } * leopph::Matrix4{ 1, 2, 3, 4 } << std::endl;

	std::cout << leopph::Vector3::Dot(leopph::Vector3{ 1, 3, -5 }, leopph::Vector3{ 4, -2, -1 }) << std::endl;
}