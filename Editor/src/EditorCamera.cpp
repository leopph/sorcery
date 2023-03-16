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
}
