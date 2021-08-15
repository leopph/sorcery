#include "CameraController.hpp"

#include <tuple>

CameraController::CameraController(leopph::Object& owner) :
	Behavior{ owner }, m_Speed{ 2.0f }, m_Sens{ 0.1f }, lastX{}, lastY{}
{
	//std::tie(lastX, lastY) = leopph::Input::GetMousePosition();
}

void CameraController::OnFrameUpdate()
{
	if (leopph::Camera::Active() == nullptr)
		return;

	leopph::Transform& camTransform = leopph::Camera::Active()->object.Transform();


	if (leopph::Input::GetKey(leopph::KeyCode::W))
		camTransform.Translate(camTransform.Forward() * m_Speed * leopph::Time::DeltaTime());

	if (leopph::Input::GetKey(leopph::KeyCode::S))
		camTransform.Translate(-camTransform.Forward() * m_Speed * leopph::Time::DeltaTime());

	if (leopph::Input::GetKey(leopph::KeyCode::D))
		camTransform.Translate(camTransform.Right() * m_Speed * leopph::Time::DeltaTime());

	if (leopph::Input::GetKey(leopph::KeyCode::A))
		camTransform.Translate(-camTransform.Right() * m_Speed * leopph::Time::DeltaTime());

	if (leopph::Input::GetKey(leopph::KeyCode::E))
		camTransform.Translate(leopph::Vector3::Up() * m_Speed * leopph::Time::DeltaTime());

	if (leopph::Input::GetKey(leopph::KeyCode::Q))
		camTransform.Translate(leopph::Vector3::Down() * m_Speed * leopph::Time::DeltaTime());


	const auto [posX, posY] = leopph::Input::GetMousePosition();
	const float diffX = posX - lastX;
	const float diffY = posY - lastY;

	camTransform.RotateGlobal(leopph::Quaternion{ leopph::Vector3::Up(), diffX * m_Sens });
	camTransform.RotateLocal(leopph::Quaternion{ leopph::Vector3::Right(), diffY * m_Sens });

	lastX = posX;
	lastY = posY;
}