#include "Parallaxer.hpp"

using leopph::Vector3;


namespace demo
{
	Parallaxer::Parallaxer(leopph::Camera* camera, std::span<Layer> layers) :
		mLayers{layers.begin(), layers.end()},
		mTargetCam{std::move(camera)},
		mPrevCamPosX{mTargetCam->get_owner()->get_position()[0]}
	{ }



	void Parallaxer::on_frame_update()
	{
		for (auto const& [speedMult, entity] : mLayers)
		{
			entity->set_position(entity->get_position() + Vector3{(mTargetCam->get_owner()->get_position()[0] - mPrevCamPosX) * speedMult, 0, 0});
		}

		mPrevCamPosX = mTargetCam->get_owner()->get_position()[0];
	}
}
