#include "Flicker.hpp"


namespace demo
{
	Flicker::Flicker(leopph::Light* light, const float on, const float out) :
		m_Light{light},
		m_OutTime{out},
		m_OnTime{on}
	{}


	auto Flicker::OnFrameUpdate() -> void
	{
		m_Time += leopph::time::DeltaTime();

		if (m_Light->IsActive())
		{
			if (m_Time >= m_OnTime)
			{
				m_Time = 0;
				m_Light->Deactivate();
			}
		}
		else
		{
			if (m_Time >= m_OutTime)
			{
				m_Time = 0;
				m_Light->Activate();
			}
		}
	}
}
