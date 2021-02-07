#include <iostream>

#include <entry.h>
#include <vector.h>
#include <matrix.h>
#include <leopphmath.h>
#include <object.h>
#include <behavior.h>
#include <pointlight.h>
#include <model.h>
#include <camera.h>
#include <timekeeping.h>

using leopph::Vector4;
using leopph::Vector3;
using leopph::Matrix3;
using leopph::Matrix4;
using leopph::Object;
using leopph::Behavior;
using leopph::Model;
using leopph::Camera;
using leopph::Time;
using leopph::Input;


class CameraMove : public Behavior
{
public:
	using Behavior::Behavior;

	void operator()()
	{
		Camera& cam = Camera::Instance();
		
		if (Input::GetKey(leopph::KeyCode::W))
			cam.Position(cam.Position() + Vector3{ 0, 0, -1 } *Time::DeltaTime());

		if (Input::GetKey(leopph::KeyCode::S))
			cam.Position(cam.Position() + Vector3{ 0, 0, 1 } *Time::DeltaTime());

		if (Input::GetKey(leopph::KeyCode::A))
			cam.Position(cam.Position() + Vector3{ -1, 0, 0 } *Time::DeltaTime());

		if (Input::GetKey(leopph::KeyCode::D))
			cam.Position(cam.Position() + Vector3{ 1, 0, 0 } *Time::DeltaTime());
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

	Object* cameraMover = Object::Create();
	cameraMover->AddBehavior<CameraMove>();
}