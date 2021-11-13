#include "CameraController.hpp"

#include <tuple>

CameraController::CameraController(leopph::Entity& owner) :
	Behavior{ owner }, m_Speed{ 2.0f }, m_Sens{ 0.1f }, lastX{}, lastY{}
{
	std::tie(lastX, lastY) = leopph::Input::GetMousePosition();
}

void CameraController::OnFrameUpdate()
{
	if (leopph::Camera::Active == nullptr)
		return;

	auto& camTransform = *leopph::Camera::Active->Entity.Transform;

	leopph::Vector3 movementVector;

	if (leopph::Input::GetKey(leopph::KeyCode::W))
		movementVector += camTransform.Forward();

	if (leopph::Input::GetKey(leopph::KeyCode::S))
		movementVector -= camTransform.Forward();

	if (leopph::Input::GetKey(leopph::KeyCode::D))
		movementVector += camTransform.Right();

	if (leopph::Input::GetKey(leopph::KeyCode::A))
		movementVector -= camTransform.Right();

	if (leopph::Input::GetKey(leopph::KeyCode::Space))
		movementVector += leopph::Vector3::Up();

	if (leopph::Input::GetKey(leopph::KeyCode::LeftControl))
		movementVector += leopph::Vector3::Down();

	movementVector.Normalize();
	camTransform.Translate(movementVector * m_Speed * leopph::Time::DeltaTime(), leopph::Space::World);

	const auto [posX, posY] = leopph::Input::GetMousePosition();
	const float diffX = posX - lastX;
	const float diffY = posY - lastY;


	short coefficient = 1;
	if (leopph::Vector3::Dot(camTransform.Up(), leopph::Vector3::Up()) < 0)
	{
		coefficient = -1;
	}

	camTransform.Rotate(leopph::Quaternion{ coefficient * leopph::Vector3::Up(), diffX * m_Sens }, leopph::Space::World);
	camTransform.Rotate(leopph::Quaternion{ leopph::Vector3::Right(), diffY * m_Sens }, leopph::Space::Local);

	lastX = posX;
	lastY = posY;
}