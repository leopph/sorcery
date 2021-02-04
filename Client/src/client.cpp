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
	std::cout << leopph::Math::ToDegrees(2 * leopph::Math::Pi()) << std::endl;
	std::cout << leopph::Math::ToRadians(180) << std::endl;
}