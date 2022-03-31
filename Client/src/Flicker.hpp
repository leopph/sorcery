#pragma once

#include "Leopph.hpp"

#include <random>


namespace demo
{
	class Flicker final : public leopph::Behavior
	{
		public:
			explicit Flicker(leopph::ComponentPtr<leopph::Light> light, float minOnTime, float maxOnTime, float minOffTim, float maxOffTim);
			auto OnFrameUpdate() -> void override;

		private:
			leopph::ComponentPtr<leopph::Light> m_Light;
			std::mt19937 m_Gen{std::random_device{}()};
			std::uniform_real_distribution<float> m_OnDist;
			std::uniform_real_distribution<float> m_OffDist;
			float m_OnTime;
			float m_OffTime;
			float m_Time{0};
	};
}
