#include "Job.hpp"

#include "../util/logger.h"

#include <atomic>
#include <utility>


namespace leopph::internal
{
	Job::Job(JobFunc func) :
		m_Id{GenId()},
		m_Func{std::move(func)}
	{ }


	auto Job::operator()() -> void
	{
		try
		{
			m_Func();
			m_Promise.set_value();
		}
		catch (...)
		{
			Logger::Instance().Debug("There was an error executing job " + std::to_string(m_Id) + ".");
		}
	}


	auto Job::Future() -> Job::FutureType
	{
		return m_Promise.get_future();	
	}


	auto Job::GenId() noexcept -> std::size_t
	{
		static std::atomic<std::size_t> id{0};
		return id++;
	}
}
