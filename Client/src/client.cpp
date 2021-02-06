#include <entry.h>
#include <vector.h>
#include <matrix.h>
#include <iostream>
#include <leopphmath.h>
#include <object.h>
#include <behavior.h>
#include <pointlight.h>
#include <model.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using leopph::Vector4;
using leopph::Vector3;
using leopph::Matrix3;
using leopph::Matrix4;
using leopph::Object;
using leopph::Behavior;
using leopph::Model;


void leopph::Init()
{
	/*Object* backpack = Object::Create();
	backpack->AddModel(Model{ "models/backpack/backpack.obj" });
	backpack->Position({ 0, 0, -5 });

	Object* light = Object::Create();
	light->AddBehavior<PointLight>();
	light->Position({ 0, 0, -1 });*/

	auto glm = glm::perspective(90.0f, 1.6f, 1.0f, 100.0f);
	auto glmp = glm::value_ptr(glm);
	for (size_t i = 0; i < 16; i++)
		std::cout << glmp[i] << ", ";
	std::cout << std::endl;

	auto leopph = Matrix4::Perspective(90.0f, 1.6f, 1.0f, 100.0f);
	auto leopphp = leopph.Data();
	for (size_t i = 0; i < 16; i++)
		std::cout << leopphp[i] << ", ";
	std::cout << std::endl;
}