#pragma once

#include <algorithm>
#include <concepts>
#include <functional>
#include <type_traits>
#include <unordered_set>

#include "../util/equal/FuncEqual.hpp"
#include "../util/hash/FuncHash.hpp"


namespace leopph
{
	template<class... T>
	class Event
	{
	public:
		template<std::convertible_to<std::decay_t<T>>... Args>
		void Dispatch(Args&&... args)
		{
			std::for_each(receivers.begin(), receivers.end(), [&](const auto& func) { func(args...); });
		}


		void RegisterListener(std::function<void(std::decay_t<T>...)> func)
		{
			receivers.emplace(std::move(func));
		}


		void RemoveListener(const std::function<void(std::decay_t<T>...)>& func)
		{
			receivers.erase(func);
		}


	private:
		std::unordered_set<std::function<void(std::decay_t<T>...)>, impl::FuncHash<void(std::decay_t<T>...)>, impl::FuncEqual<void(std::decay_t<T>...)>> receivers{};
	};
}
