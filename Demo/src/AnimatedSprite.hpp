#pragma once

#include <Leopph.hpp>
#include <span>
#include <vector>


namespace demo
{
	/*class AnimatedSprite final : public leopph::Behavior
	{
		public:
			enum class AnimationMode
			{
				Loop,
				Bounce
			};


			AnimatedSprite(std::span<leopph::ComponentPtr<leopph::ImageSprite>> sprites, AnimationMode animMode, float speed);
			AnimatedSprite(AnimatedSprite const& other);

			void on_frame_update() override;

			void OnAttach() override;
			void OnDetach() override;

		private:
			std::vector<leopph::ComponentPtr<leopph::ImageSprite>> m_Sprites;
			AnimationMode m_AnimMode;
			float m_Speed;
			float m_TimeDelta{0};
			int m_FrameInd{0};
			// This will be added to the current frame index to get the new index.
			int m_IndGradient{1};
	};*/
}
