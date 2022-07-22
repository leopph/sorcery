#pragma once

#include "LeopphApi.hpp"


namespace leopph
{
	namespace internal
	{
		// Program Or Explicit Lifetime Object.
		// Base class for objects owned by LeopphEngine whose lifetime is the lifetime of the program, unless explicitly deleted.
		// For this reason, Poelos must NEVER be created using automatic storage duration.
		class Poelo
		{
			public:
				Poelo(Poelo const& other) = delete;
				Poelo& operator=(Poelo const& other) = delete;

				Poelo(Poelo&& other) noexcept = delete;
				Poelo& operator=(Poelo&& other) noexcept = delete;

				virtual ~Poelo() = default;

			protected:
				// Poelos on creation give ownership of themselves to the engine.
				// Do NOT instantiate them using automatic storage duration.
				LEOPPHAPI Poelo();
		};
	}


	// Explicitly destroys the Poelo object.
	LEOPPHAPI void Destroy(internal::Poelo const* poelo);
}
