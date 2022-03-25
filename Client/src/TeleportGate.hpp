#pragma once

#include "SceneSwitcher.hpp"

#include <Leopph.hpp>
#include <vector>


namespace demo
{
	class TeleportGate final : public leopph::Behavior
	{
		public:
			struct PortPoint
			{
				// The Transform's position is the center of activation
				leopph::ComponentPtr<leopph::Transform> ActivationPoint;
				// The distance the player has to be from the ActivationPoint
				// to trigger a scene change
				float Distance;
				// The scene to change to when triggered
				SceneSwitcher::Scene Scene;
			};


			explicit TeleportGate(leopph::Entity* player, leopph::ComponentPtr<SceneSwitcher> sceneSwitcher);
			auto OnFrameUpdate() -> void override;
			auto AddPortPoint(PortPoint const& portPoint) -> void;

		private:
			leopph::Entity* m_Player;
			leopph::ComponentPtr<SceneSwitcher> m_SceneSwitcher{nullptr};
			std::vector<PortPoint> m_PortPoints;
	};
}
