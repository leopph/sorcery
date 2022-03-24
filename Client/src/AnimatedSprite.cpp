#include "AnimatedSprite.hpp"


namespace demo
{
	AnimatedSprite::AnimatedSprite(std::span<leopph::ImageSprite*> sprites, AnimationMode const animMode, float const speed) :
		m_Sprites{sprites.begin(), sprites.end()},
		m_AnimMode{animMode},
		m_Speed{speed}
	{ }


	auto AnimatedSprite::OnFrameUpdate() -> void
	{
		m_TimeDelta += leopph::time::DeltaTime();

		if (m_TimeDelta < 1 / m_Speed)
		{
			return;
		}

		m_TimeDelta = 0;

		switch (m_AnimMode)
		{
			case AnimationMode::Loop:
			{
				m_Sprites[m_FrameInd]->Deactivate();
				m_FrameInd = (m_FrameInd + m_IndGradient) % m_Sprites.size();
				m_Sprites[m_FrameInd]->Activate();
				break;
			}

			case AnimationMode::Bounce:
			{
				m_Sprites[m_FrameInd]->Deactivate();
				m_FrameInd += m_IndGradient;
				m_Sprites[m_FrameInd]->Activate();
				m_IndGradient *= m_FrameInd == 0 || m_FrameInd == m_Sprites.size() - 1 ? -1 : 1;
				break;
			}
		}
	}


	auto AnimatedSprite::Attach(leopph::Entity* entity) -> void
	{
		Behavior::Attach(entity);

		for (auto const sprite : m_Sprites)
		{
			sprite->Attach(Entity());
			sprite->Deactivate();
		}

		m_Sprites[0]->Activate();
	}


	auto AnimatedSprite::Detach() -> void
	{
		Behavior::Detach();

		for (auto const sprite : m_Sprites)
		{
			sprite->Detach();
		}
	}
}
