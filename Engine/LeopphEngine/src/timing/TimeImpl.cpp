#include "timing/TimeImpl.hpp"


namespace leopph::internal
{
	auto TimeImpl::Instance() -> TimeImpl&
	{
		return s_Instance;
	}


	auto TimeImpl::DeltaTime() const -> float
	{
		return m_LastFrameDeltaTime.count();
	}


	auto TimeImpl::FullTime() const -> float
	{
		return m_FullTime.count();
	}


	auto TimeImpl::OnEventReceived(EventParamType) -> void
	{
		auto const currentTime{Clock::now()};
		m_LastFrameDeltaTime = currentTime - m_LastFrameCompletionTime;
		m_LastFrameCompletionTime = currentTime;
		m_FullTime += m_LastFrameDeltaTime;
	}


	TimeImpl TimeImpl::s_Instance{};
}
