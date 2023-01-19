#pragma once

#include <memory>
#include <utility>
#include <vector>


namespace leopph
{
	template<class... EventParams>
	class Event
	{
	public:
		using FreeHandlerType = void(*)(EventParams...);

		template<class T>
		using MemberHandlerType = void(*)(T*, EventParams...);



	private:
		class EventHandler
		{
		public:
			virtual void invoke(EventParams&& ...params) const = 0;
			virtual bool equals(void* explicitThis, void* handler) const = 0;
			virtual ~EventHandler() = default;
		};


		class FreeEventHandler : public EventHandler
		{
		private:
			FreeHandlerType mHandler;

		public:
			explicit FreeEventHandler(FreeHandlerType const handler) :
				mHandler{ handler }
			{}


			void invoke(EventParams&& ...params) const override
			{
				mHandler(std::forward<EventParams>(params)...);
			}


			bool equals([[maybe_unused]] void* const explicitThis, void* const handler) const override
			{
				return reinterpret_cast<void*>(mHandler) == handler;
			}
		};



		template<class T>
		class MemberEventHandler : public EventHandler
		{
		private:
			MemberHandlerType<T> mHandler;
			T* mHandlerExplicitThis;

		public:
			MemberEventHandler(T* const handlerExplicitThis, MemberHandlerType<T> handler) :
				mHandler{ handler }, mHandlerExplicitThis{ handlerExplicitThis }
			{}


			void invoke(EventParams&& ...params) const override
			{
				mHandler(mHandlerExplicitThis, std::forward<EventParams>(params)...);
			}


			bool equals(void* const explicitThis, void* const handler) const override
			{
				return mHandlerExplicitThis == reinterpret_cast<T*>(explicitThis) && mHandler == reinterpret_cast<MemberHandlerType<T>>(handler);
			}
		};



		std::vector<std::unique_ptr<EventHandler>> mHandlers;



	public:
		void invoke(EventParams&& ...params) const
		{
			for (auto const& handler : mHandlers)
			{
				handler->invoke(std::forward<EventParams>(params)...);
			}
		}


		void add_handler(FreeHandlerType const handler)
		{
			mHandlers.emplace_back(std::make_unique<FreeEventHandler>(handler));
		}


		void remove_handler(FreeHandlerType const handler)
		{
			std::erase_if(mHandlers, [handler](EventHandler const& eventHandler)
			{
				return eventHandler->equals(nullptr, handler);
			});
		}


		template<class T>
		void add_handler(T* const explicitThis, MemberHandlerType<T> const handler)
		{
			mHandlers.emplace_back(std::make_unique<MemberEventHandler<T>>(explicitThis, handler));
		}


		template<class T>
		void remove_handler(T* const explicitThis, MemberHandlerType<T> const handler)
		{
			std::erase_if(mHandlers, [explicitThis, handler](EventHandler const& eventHandler)
			{
				return eventHandler->equals(reinterpret_cast<void*>(explicitThis), reinterpret_cast<void*>(handler));
			});
		}


		void operator()(EventParams&& ...params) const
		{
			invoke(std::forward<EventParams>(params)...);
		}


		void operator+=(FreeHandlerType const handler)
		{
			add_handler(handler);
		}


		void operator-=(FreeHandlerType const handler)
		{
			remove_handler(handler);
		}
	};


	template<class... EventParams>
	class GuardedEventReference
	{
	private:
		Event<EventParams...>& mEvent;

	public:
		using FreeHandlerType = typename Event<EventParams...>::FreeHandlerType;

		template<class T>
		using MemberHandlerType = typename Event<EventParams...>::template MemberHandlerType<T>;


		GuardedEventReference(Event<EventParams...>& event) :
			mEvent{ event }
		{}


		void add_handler(FreeHandlerType const handler)
		{
			mEvent.add_handler(handler);
		}


		void remove_handler(FreeHandlerType const handler)
		{
			mEvent.remove_handler(handler);
		}


		template<class T>
		void add_handler(T* explicitThis, MemberHandlerType<T> const handler)
		{
			mEvent.add_handler(explicitThis, handler);
		}


		template<class T>
		void remove_handler(T* explicitThis, MemberHandlerType<T> handler)
		{
			mEvent.remove_handler(explicitThis, handler);
		}


		void operator+=(FreeHandlerType const handler)
		{
			mEvent.add_handler(handler);
		}


		void operator-=(FreeHandlerType handler)
		{
			mEvent.remove_handler(handler);
		}
	};
}