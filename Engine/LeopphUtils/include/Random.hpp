#pragma once

#include "LeopphApi.hpp"
#include "Types.hpp"

#include <array>


namespace leopph
{
	class XorShift128
	{
		public:
			// s0, s1, s2 and s3 may not be all zero at the same time
			LEOPPHAPI XorShift128(u32 s0, u32 s1, u32 s2, u32 s3);

			// At least one element must be non-zero
			explicit LEOPPHAPI XorShift128(std::array<u32, 4> seeds);

			[[nodiscard]] LEOPPHAPI auto operator()() -> u32;

		private:
			std::array<u32, 4> m_State;
	};
}
