#pragma once

#include <memory>


namespace leopph::internal
{
	// Base class for Poelos to join all Poelo template instances together.
	class PoeloBase
	{
		public:
			PoeloBase(const PoeloBase& other) = delete;
			auto operator=(const PoeloBase& other) -> PoeloBase& = delete;

			PoeloBase(PoeloBase&& other) noexcept = delete;
			auto operator=(PoeloBase&& other) noexcept -> PoeloBase& = delete;

			virtual ~PoeloBase() = default;

		protected:
			// Pass an owning pointer to your instance to hand its ownership over to the engine.
			static auto Store(std::unique_ptr<PoeloBase> poeloBase) -> void;
			// Pass a pointer to an instance to call the destructor on it and free its memory.
			static auto Destroy(const PoeloBase* poeloBase) -> void;

			PoeloBase() = default;
	};
}
