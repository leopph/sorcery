#include "EditorCamera.hpp"

namespace leopph::editor {
auto EditorCamera::GetPosition() const noexcept -> Vector3 {
	return position;
}


auto EditorCamera::GetForwardAxis() const noexcept -> Vector3 {
	return orientation.Rotate(Vector3::Forward());
}


auto EditorCamera::GetNearClipPlane() const noexcept -> float {
	return nearClip;
}


auto EditorCamera::GetFarClipPlane() const noexcept -> float {
	return farClip;
}


auto EditorCamera::GetHorizontalOrthographicSize() const -> float {
	return 0;
}


auto EditorCamera::GetHorizontalPerspectiveFov() const -> float {
	return fovHorizDeg;
}


auto EditorCamera::GetType() const -> Type {
	return Type::Perspective;
}


EditorCamera::EditorCamera(Vector3 const& position, Quaternion const& orientation, float const nearClip, float const farClip, float const fovHorizDeg) :
	position{ position }, orientation{ orientation }, nearClip{ nearClip }, farClip{ farClip }, fovHorizDeg{ fovHorizDeg } { }


auto EditorCamera::GetFrustum(float const aspectRatio) const -> Frustum {
	auto const horizFov{ GetHorizontalPerspectiveFov() };
	auto const nearClipPlane{ GetNearClipPlane() };
	auto const farClipPlane{ GetFarClipPlane() };

	auto const tanHalfHorizFov{ std::tan(ToRadians(horizFov) / 2.0f) };
	auto const tanHalfVertFov{ std::tan(ToRadians(HorizontalPerspectiveFovToVertical(horizFov, aspectRatio)) / 2.0f) };

	auto const xn = nearClipPlane * tanHalfHorizFov;
	auto const xf = farClipPlane * tanHalfHorizFov;
	auto const yn = nearClipPlane * tanHalfVertFov;
	auto const yf = farClipPlane * tanHalfVertFov;

	return Frustum
	{
		.rightTopNear = Vector3{ xn, yn, nearClipPlane },
		.leftTopNear = Vector3{ -xn, yn, nearClipPlane },
		.leftBottomNear = Vector3{ -xn, -yn, nearClipPlane },
		.rightBottomNear = Vector3{ xn, -yn, nearClipPlane },
		.rightTopFar = Vector3{ xf, yf, farClipPlane },
		.leftTopFar = Vector3{ -xf, yf, farClipPlane },
		.leftBottomFar = Vector3{ -xf, -yf, farClipPlane },
		.rightBottomFar = Vector3{ xf, -yf, farClipPlane },
	};
}
}
