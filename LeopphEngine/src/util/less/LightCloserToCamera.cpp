#include "LightCloserToCamera.hpp"

#include "../../components/Camera.hpp"
#include "../../entity/Entity.hpp"


namespace leopph::impl
{
	bool LightCloserToCamera::operator()(const Light* const left, const Light* const right) const
	{
		const auto& camPosition{ Camera::Active()->entity.Transform->Position() };
		const auto& leftDistance{ Vector3::Distance(camPosition, left->entity.Transform->Position()) };
		const auto& rightDistance{ Vector3::Distance(camPosition, right->entity.Transform->Position()) };

		if (leftDistance != rightDistance)
		{
			return leftDistance < rightDistance;
		}

		return left < right;
	}
}