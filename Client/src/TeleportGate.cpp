#include "TeleportGate.hpp"

#include <algorithm>


namespace demo
{
	TeleportGate::TeleportGate(leopph::Entity* player, leopph::ComponentPtr<SceneSwitcher> sceneSwitcher) :
		m_Player{player},
		m_SceneSwitcher{std::move(sceneSwitcher)}
	{}


	auto TeleportGate::OnFrameUpdate() -> void
	{
		std::ranges::for_each(m_PortPoints, [this](auto const& portPoint)
		{
			if (leopph::Vector3::Distance(m_Player->Transform()->Position(), portPoint.ActivationPoint->Position()) < portPoint.Distance)
			{
				m_SceneSwitcher->ActivateScene(portPoint.Scene);
			}
		});
	}


	auto TeleportGate::AddPortPoint(PortPoint const& portPoint) -> void
	{
		m_PortPoints.push_back(portPoint);
	}
}
