#pragma once

#include "Leopph.hpp"


namespace demo
{
	class Flicker final : public leopph::Behavior
	{
		public:
			Flicker(leopph::ComponentPtr<leopph::Light> light, float on, float out);

			auto OnFrameUpdate() -> void override;

		private:
			leopph::ComponentPtr<leopph::Light> m_Light;
			float m_OutTime;
			float m_OnTime;
			float m_Time{0};
	};
}
