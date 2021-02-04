#include <entry.h>
#include <vector.h>
#include <matrix.h>
#include <iostream>
#include <leopphmath.h>
#include <object.h>
#include <behavior.h>

using leopph::Vector3;
using leopph::Matrix3;
using leopph::Matrix4;
using leopph::Object;
using leopph::Behavior;


class Print : public Behavior
{
private:
	Matrix3 m_Mat{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };

public:
	void operator()()
	{
		m_Mat *= m_Mat.Transposed();
		std::cout << m_Mat << std::endl;
	}
};


void leopph::Init()
{
	Object* object = leopph::Object::Create();
	object->AddBehavior<Print>();
}