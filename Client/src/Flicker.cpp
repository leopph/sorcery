#include "Flicker.hpp"

using leopph::time::DeltaTime;


namespace demo
{
	Flicker::Flicker(leopph::ComponentPtr<leopph::Light> light, float const minOnTime, float const maxOnTime, float const minOffTime, float const maxOffTime) :
		m_Light{std::move(light)},
		m_OnDist{minOnTime, maxOnTime},
		m_OffDist{minOffTime, maxOffTime},
		m_OnTime{m_OnDist(m_Gen)},
		m_OffTime{m_OffDist(m_Gen)}
	{ }


	auto Flicker::OnFrameUpdate() -> void
	{
		m_Time += DeltaTime();

		if (m_Light->Active())
		{
			if (m_Time > m_OnTime)
			{
				m_Light->Deactivate();
				m_Time = 0;
				m_OnTime = m_OnDist(m_Gen);
			}
		}
		else
		{
			if (m_Time > m_OffTime)
			{
				m_Light->Activate();
				m_Time = 0;
				m_OffTime = m_OffDist(m_Gen);
			}
		}
	}
}
