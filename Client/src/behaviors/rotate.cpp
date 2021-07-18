#include "rotate.h"

Rotate::Rotate() :
	m_Axis{ 0, 1, 0 },
	m_Angle{ 10.f }
{}

void Rotate::OnFrameUpdate()
{
	Object().Transform().RotateGlobal(leopph::Quaternion{ m_Axis, m_Angle * leopph::Time::DeltaTime() });
}