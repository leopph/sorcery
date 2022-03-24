#pragma once

#include <Leopph.hpp>


namespace demo
{
	class CharacterController2D final : public leopph::Behavior
	{
		public:
			CharacterController2D(leopph::Transform* target, float speed, float runMult, float walkMult);

			auto OnFrameUpdate() -> void override;

		private:
			leopph::Transform* m_Target;
			float m_Speed;
			float m_RunMult;
			float m_WalkMult;
	};
}
