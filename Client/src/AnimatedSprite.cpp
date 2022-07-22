#include "AnimatedSprite.hpp"

using leopph::ImageSprite;
using leopph::ComponentPtr;


namespace demo
{
	AnimatedSprite::AnimatedSprite(std::span<ComponentPtr<ImageSprite>> sprites, AnimationMode const animMode, float const speed) :
		m_Sprites{sprites.begin(), sprites.end()},
		m_AnimMode{animMode},
		m_Speed{speed}
	{ }


	AnimatedSprite::AnimatedSprite(AnimatedSprite const& other) :
		Behavior{other},
		m_Sprites{
			[&other]
			{
				std::vector<ComponentPtr<ImageSprite>> ret;
				for (auto const& spritePtr : other.m_Sprites)
				{
					ret.push_back(leopph::CreateComponent<ImageSprite>(*spritePtr));
				}
				return ret;
			}()
		},
		m_AnimMode{other.m_AnimMode},
		m_Speed{other.m_Speed},
		m_TimeDelta{other.m_TimeDelta},
		m_FrameInd{other.m_FrameInd},
		m_IndGradient{other.m_IndGradient}
	{ }


	void AnimatedSprite::OnFrameUpdate()
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


	void AnimatedSprite::OnAttach()
	{
		Behavior::OnAttach();

		for (auto const& sprite : m_Sprites)
		{
			sprite->Attach(Owner());
			sprite->Deactivate();
		}

		m_Sprites[0]->Activate();
	}


	void AnimatedSprite::OnDetach()
	{
		Behavior::OnDetach();

		for (auto const& sprite : m_Sprites)
		{
			sprite->Detach();
		}
	}
}
