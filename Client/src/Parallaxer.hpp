#pragma once

#include <Leopph.hpp>
#include <span>
#include <vector>


namespace demo
{
	class Parallaxer final : public leopph::Behavior
	{
		public:
			struct Layer
			{
				// The layer will be moved with SpeedMult * the camera's speed
				float SpeedMult;
				leopph::Transform* Transform;
			};


			explicit Parallaxer(leopph::ComponentPtr<leopph::Camera> camera, std::span<Layer> layers = {});

			auto OnFrameUpdate() -> void override;

		private:
			std::vector<Layer> m_Layers;
			leopph::ComponentPtr<leopph::Camera> m_TargetCamera;
			float m_PrevCamPosX;
	};
}
