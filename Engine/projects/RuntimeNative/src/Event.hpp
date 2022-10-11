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
			explicit FreeEventHandler(FreeHandlerType handler);
			void invoke(EventParams&& ...params) const override;
			bool equals(void* explicitThis, void* handler) const override;
		};


		template<class T>
		class MemberEventHandler : public EventHandler
		{
		private:
			MemberHandlerType<T> mHandler;
			T* mHandlerExplicitThis;

		public:
			MemberEventHandler(T* handlerExplicitThis, MemberHandlerType<T> handler);
			void invoke(EventParams&& ...params) const override;
			bool equals(void* explicitThis, void* handler) const override;
		};


		std::vector<std::unique_ptr<EventHandler>> mHandlers;

	public:
		void invoke(EventParams&& ...params) const;
		void add_handler(FreeHandlerType handler);
		void remove_handler(FreeHandlerType handler);

		template<class T>
		void add_handler(T* explicitThis, MemberHandlerType<T> handler);

		template<class T>
		void remove_handler(T* explicitThis, MemberHandlerType<T> handler);

		void operator()(EventParams&& ...params) const;
		void operator+=(FreeHandlerType handler);
		void operator-=(FreeHandlerType handler);
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

		void add_handler(FreeHandlerType handler);
		void remove_handler(FreeHandlerType handler);

		template<class T>
		void add_handler(T* explicitThis, MemberHandlerType<T> handler);

		template<class T>
		void remove_handler(T* explicitThis, MemberHandlerType<T> handler);

		void operator+=(FreeHandlerType handler);
		void operator-=(FreeHandlerType handler);

		GuardedEventReference(Event<EventParams...>& event);
	};


	template<class... EventParams>
	Event<EventParams...>::FreeEventHandler::FreeEventHandler(FreeHandlerType const handler) :
		mHandler{handler}
	{}


	template<class... EventParams>
	void Event<EventParams...>::FreeEventHandler::invoke(EventParams&& ...params) const
	{
		mHandler(std::forward<EventParams>(params)...);
	}


	template<class... EventParams>
	bool Event<EventParams...>::FreeEventHandler::equals(void* explicitThis, void* handler) const
	{
		return reinterpret_cast<void*>(mHandler) == handler;
	}


	template<class... EventParams>
	template<class T>
	Event<EventParams...>::MemberEventHandler<T>::MemberEventHandler(T* handlerExplicitThis, MemberHandlerType<T> handler) :
		mHandler{handler}, mHandlerExplicitThis{handlerExplicitThis}
	{}


	template<class... EventParams>
	template<class T>
	void Event<EventParams...>::MemberEventHandler<T>::invoke(EventParams&& ...params) const
	{
		mHandler(mHandlerExplicitThis, std::forward<EventParams>(params)...);
	}


	template<class... EventParams>
	template<class T>
	bool Event<EventParams...>::MemberEventHandler<T>::equals(void* explicitThis, void* handler) const
	{
		return mHandlerExplicitThis == reinterpret_cast<T*>(explicitThis) && mHandler == reinterpret_cast<MemberHandlerType<T>>(handler);
	}


	template<class... EventParams>
	void Event<EventParams...>::invoke(EventParams&& ...params) const
	{
		for (auto const& handler : mHandlers)
		{
			handler->invoke(std::forward<EventParams>(params)...);
		}
	}


	template<class... EventParams>
	void Event<EventParams...>::add_handler(FreeHandlerType const handler)
	{
		mHandlers.emplace_back(std::make_unique<FreeEventHandler>(handler));
	}


	template<class... EventParams>
	void Event<EventParams...>::remove_handler(FreeHandlerType const handler)
	{
		std::erase_if(mHandlers, [handler](EventHandler const& eventHandler)
		{
			return eventHandler->equals(nullptr, handler);
		});
	}


	template<class... EventParams>
	template<class T>
	void Event<EventParams...>::add_handler(T* const explicitThis, MemberHandlerType<T> const handler)
	{
		mHandlers.emplace_back(std::make_unique<MemberEventHandler<T>>(explicitThis, handler));
	}


	template<class... EventParams>
	template<class T>
	void Event<EventParams...>::remove_handler(T* const explicitThis, MemberHandlerType<T> const handler)
	{
		std::erase_if(mHandlers, [explicitThis, handler](EventHandler const& eventHandler)
		{
			return eventHandler->equals(reinterpret_cast<void*>(explicitThis), reinterpret_cast<void*>(handler));
		});
	}


	template<class... EventParams>
	void Event<EventParams...>::operator()(EventParams&& ...params) const
	{
		invoke(std::forward<EventParams>(params)...);
	}


	template<class... EventParams>
	void Event<EventParams...>::operator+=(FreeHandlerType const handler)
	{
		add_handler(handler);
	}


	template<class... EventParams>
	void Event<EventParams...>::operator-=(FreeHandlerType const handler)
	{
		remove_handler(handler);
	}


	template<class... EventParams>
	GuardedEventReference<EventParams...>::GuardedEventReference(Event<EventParams...>& event) :
		mEvent{event}
	{}


	template<class... EventParams>
	void GuardedEventReference<EventParams...>::add_handler(FreeHandlerType const handler)
	{
		mEvent.add_handler(handler);
	}


	template<class... EventParams>
	void GuardedEventReference<EventParams...>::remove_handler(FreeHandlerType const handler)
	{
		mEvent.remove_handler(handler);
	}


	template<class... EventParams>
	template<class T>
	void GuardedEventReference<EventParams...>::add_handler(T* const explicitThis, MemberHandlerType<T> const handler)
	{
		mEvent.add_handler(explicitThis, handler);
	}


	template<class... EventParams>
	template<class T>
	void GuardedEventReference<EventParams...>::remove_handler(T* const explicitThis, MemberHandlerType<T> const handler)
	{
		mEvent.remove_handler(explicitThis, handler);
	}


	template<class... EventParams>
	void GuardedEventReference<EventParams...>::operator+=(FreeHandlerType const handler)
	{
		mEvent.add_handler(handler);
	}


	template<class... EventParams>
	void GuardedEventReference<EventParams...>::operator-=(FreeHandlerType const handler)
	{
		mEvent.remove_handler(handler);
	}
}