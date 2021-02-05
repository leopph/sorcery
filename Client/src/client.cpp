#include <entry.h>
#include <vector.h>
#include <matrix.h>
#include <iostream>
#include <leopphmath.h>
#include <object.h>
#include <behavior.h>
#include <pointlight.h>
#include <model.h>

using leopph::Vector3;
using leopph::Matrix3;
using leopph::Matrix4;
using leopph::Object;
using leopph::Behavior;
using leopph::Model;


class Print : public Behavior
{
public:
	using Behavior::Behavior;

	void operator()()
	{
		std::cout << Matrix4::Translate(Vector3{ 1, 2, 3 }) << std::endl;
	}
};



void leopph::Init()
{
	Object* backpack = Object::Create();
	std::cout << "backpack obj created" << std::endl;

	backpack->AddModel(Model{ "models/backpack/backpack.obj" });
	std::cout << "backpack model added" << std::endl;

	backpack->Position({ 0, 0, -5 });
	std::cout << "backpack pos set" << std::endl;

	Object* light = Object::Create();
	std::cout << "light obj created" << std::endl;

	light->AddBehavior<PointLight>();
	std::cout << "light comp added" << std::endl;

	light->Position({ 0, 0, -3 });
	std::cout << "light pos set" << std::endl;
}