#include "TimeImpl.hpp"


namespace leopph::internal
{
	TimeImpl& TimeImpl::Instance()
	{
		return s_Instance;
	}


	float TimeImpl::DeltaTime() const
	{
		return m_LastFrameDeltaTime.count();
	}


	float TimeImpl::FullTime() const
	{
		return m_FullTime.count();
	}


	void TimeImpl::OnEventReceived(EventParamType)
	{
		auto const currentTime{Clock::now()};
		m_LastFrameDeltaTime = currentTime - m_LastFrameCompletionTime;
		m_LastFrameCompletionTime = currentTime;
		m_FullTime += m_LastFrameDeltaTime;
	}


	TimeImpl TimeImpl::s_Instance{};
}
