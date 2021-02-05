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
		Matrix3 m{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		m.Transpose();

		for (size_t i = 0; i < 9; i++)
			std::cout << m.Data()[i] << " ";
		std::cout << std::endl;
	}
};


void leopph::Init()
{
	Object* backpack = Object::Create();
	backpack->AddModel(Model{ "models/backpack/backpack.obj" });
	backpack->Position({ 0, 0, -5 });

	Object* light = Object::Create();
	light->AddBehavior<PointLight>();
	light->Position({ 0, 0, -1 });

	//Object* tmp = Object::Create();
	//tmp->AddBehavior<Print>();
}