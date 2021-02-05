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
	Object* tetej = Object::Create();
	tetej->AddModel(Model{ "models/samy/tetej/tetej_spec.obj" });
	tetej->Position({ 0, -5, 0 });

	Object* light = Object::Create();
	light->AddBehavior<PointLight>();
	light->Position({ 0, -5, 0 });
}