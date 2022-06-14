#include "Worker.hpp"


namespace leopph::internal
{
	Worker::Worker(Worker&& other) noexcept :
		std::thread{static_cast<std::thread&&>(other)},
		m_Label{other.m_Label}
	{}


	auto Worker::operator=(Worker&& other) noexcept -> Worker&
	{
		m_Label = other.m_Label;
		std::thread::operator=(std::move(other));
		return *this;
	}
}
