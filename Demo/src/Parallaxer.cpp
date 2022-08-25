#include "Parallaxer.hpp"

using leopph::Vector3;


namespace demo
{
	Parallaxer::Parallaxer(leopph::CameraNode* camera, std::span<Layer> layers) :
		mLayers{layers.begin(), layers.end()},
		mTargetCam{camera},
		mPrevCamPosX{mTargetCam->get_position()[0]}
	{ }



	void Parallaxer::on_frame_update()
	{
		for (auto const& [speedMult, entity] : mLayers)
		{
			entity->set_position(entity->get_position() + Vector3{(mTargetCam->get_position()[0] - mPrevCamPosX) * speedMult, 0, 0});
		}

		mPrevCamPosX = mTargetCam->get_position()[0];
	}
}
