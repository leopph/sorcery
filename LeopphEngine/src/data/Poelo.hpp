#pragma once

#include "PoeloBase.hpp"


namespace leopph::internal
{
	// Program Or Explicit Lifetime Object.
	// Base class for objects owned by LeopphEngine whose lifetime is the lifetime of the program, unless explicitly deleted.
	template<typename T>
	class Poelo : PoeloBase
	{
		public:
			Poelo(const Poelo& other) = delete;
			auto operator=(const Poelo& other) -> Poelo& = delete;

			Poelo(Poelo&& other) noexcept = delete;
			auto operator=(Poelo&& other) noexcept -> Poelo& = delete;

			~Poelo() override = default;

		protected:
			// Pass an instance pointer to give its ownership to the engine.
			// This should usually be called in factory functions.
			static auto TakeOwnership(Poelo* poelo) -> void;
			// Pass an instance pointer to call the destructor on the instance and free its memory.
			// This should usually be called in deleter functions.
			static auto Destroy(const Poelo* poelo) -> void;

			Poelo() = default;
	};


	template<typename T>
	auto Poelo<T>::TakeOwnership(Poelo<T>* poelo) -> void
	{
		Store(std::unique_ptr<PoeloBase>{poelo});
	}


	template<typename T>
	auto Poelo<T>::Destroy(const Poelo* poelo) -> void
	{
		PoeloBase::Destroy(poelo);
	}
}
