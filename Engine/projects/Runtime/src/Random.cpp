#include "Random.hpp"


namespace leopph
{
	XorShift128::XorShift128(u32 const s0, u32 const s1, u32 const s2, u32 const s3) :
		m_State{s0, s1, s2, s3}
	{ }


	XorShift128::XorShift128(std::array<u32, 4> seeds) :
		m_State{seeds}
	{ }


	u32 XorShift128::operator()()
	{
		auto t = m_State[3];
		auto s = m_State[0];

		m_State[3] = m_State[2];
		m_State[2] = m_State[1];
		m_State[1] = s;

		t ^= t << 11;
		t ^= t >> 8;

		return m_State[0] = t ^ s ^ (s >> 19);
	}
}
