#include <entry.h>
#include <vector.h>
#include <matrix.h>
#include <iostream>

using leopph::Vector3;
using leopph::Matrix3;


void leopph::Init()
{
	Vector3 v1{ 3, 4, 2 };
	Matrix3 m1{ 13, 9, 7, 8, 7, 4, 6, 4, 0 };
	std::cout << v1 * m1 << std::endl << std::endl;

	Matrix3 m2{ 0, 1, 0, 1, 0, 0, 0, 0, 1 };
	Vector3 v2{ 2, 1, 3 };
	std::cout << m2 * v2 << std::endl;
}