#include "Flicker.hpp"


namespace demo
{
	Flicker::Flicker(leopph::ComponentPtr<leopph::Light> light, float const on, float const out) :
		m_Light{std::move(light)},
		m_OutTime{out},
		m_OnTime{on}
	{}


	auto Flicker::OnFrameUpdate() -> void
	{
		m_Time += leopph::time::DeltaTime();

		if (m_Light->Active())
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
