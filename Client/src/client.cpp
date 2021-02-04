#include <entry.h>
#include <vector.h>
#include <matrix.h>
#include <iostream>

using leopph::Vector3;
using leopph::Matrix3;
using leopph::Matrix4;


void leopph::Init()
{
	Matrix3 m1{ 1,2,3,4,5,6,7,8,9 };
	Matrix3 m2{ 1,2,3,4,5,6,7,8,9 };

	std::cout << m1 * m2 << std::endl;
}