#include "LightCloserToCamera.hpp"

#include "../../components/Camera.hpp"
#include "../../entity/Entity.hpp"


namespace leopph::internal
{
	auto LightCloserToCamera::operator()(const Light* const left, const Light* const right) const -> bool
	{
		const auto& camPosition{Camera::Active()->Entity()->Transform()->Position()};
		const auto& leftDistance{Vector3::Distance(camPosition, left->Entity()->Transform()->Position())};
		const auto& rightDistance{Vector3::Distance(camPosition, right->Entity()->Transform()->Position())};

		if (leftDistance != rightDistance)
		{
			return leftDistance < rightDistance;
		}

		return left < right;
	}
}
