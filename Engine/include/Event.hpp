#pragma once

#include "LeopphApi.hpp"

#include <concepts>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>


namespace leopph
{
	class Event
	{
	protected:
		Event() = default;
		Event(Event const& other) = default;
		Event(Event&& other) noexcept = default;

	public:
		virtual ~Event() = 0;

	protected:
		Event& operator=(Event const& other) = default;
		Event& operator=(Event&& other) noexcept = default;

	};



	class EventReceiverBase
	{
	public:
		virtual void internal_handle_event(Event const& event) = 0;


	protected:
		EventReceiverBase() = default;
		EventReceiverBase(EventReceiverBase const& other) = default;
		EventReceiverBase(EventReceiverBase&& other) noexcept = default;

	public:
		virtual ~EventReceiverBase() = default;

		EventReceiverBase& operator=(EventReceiverBase const& other) = default;
		EventReceiverBase& operator=(EventReceiverBase&& other) noexcept = default;
	};



	class EventManager
	{
	public:
		[[nodiscard]] LEOPPHAPI static EventManager& get_instance();

		template<std::derived_from<Event> EventType, class... Args>
		EventManager& send(Args&&... args);

		void register_receiver(std::type_index const& typeIndex, EventReceiverBase* receiver);
		void unregister_receiver(std::type_index const& typeIndex, EventReceiverBase const* receiver);


	private:
		EventManager() = default;

	public:
		EventManager(EventManager const& other) = delete;
		EventManager(EventManager&& other) = delete;

		void operator=(EventManager const& other) = delete;
		void operator=(EventManager&& other) = delete;

	private:
		~EventManager() = default;

		std::unordered_map<std::type_index, std::vector<EventReceiverBase*>> mReceivers;
	};



	template<std::derived_from<Event> EventType>
	class EventReceiver : public EventReceiverBase
	{
	public:
		void internal_handle_event(Event const& event) override;
	private:
		virtual void on_event(EventType const& event) = 0;


	protected:
		EventReceiver();
		EventReceiver(EventReceiver const& other);
		EventReceiver(EventReceiver&& other) noexcept;

	public:
		~EventReceiver() override;

	protected:
		EventReceiver& operator=(EventReceiver const& other) = default;
		EventReceiver& operator=(EventReceiver&& other) noexcept = default;
	};


	template<std::derived_from<Event> EventType>
	class EventReceiverHandle : public EventReceiver<EventType>
	{
	public:
		explicit EventReceiverHandle(std::function<void(EventType const&)> callback);

	private:
		void on_event(EventType const& event) override;

		std::function<void(EventType const&)> mCallback;
	};



	template<std::derived_from<Event> EventType, class... Args>
	EventManager& EventManager::send(Args&&... args)
	{
		if (auto const it = mReceivers.find(typeid(EventType)); it != std::end(mReceivers))
		{
			EventType event{std::forward<Args>(args)...};

			for (auto* const receiver : it->second)
			{
				receiver->internal_handle_event(event);
			}
		}

		return *this;
	}


	template<std::derived_from<Event> EventType>
	EventReceiver<EventType>::EventReceiver()
	{
		EventManager::get_instance().register_receiver(typeid(EventType), this);
	}



	template<std::derived_from<Event> EventType>
	EventReceiver<EventType>::EventReceiver(EventReceiver const&) :
		EventReceiver{}
	{}



	template<std::derived_from<Event> EventType>
	EventReceiver<EventType>::EventReceiver(EventReceiver&& other) noexcept :
		EventReceiver{other}
	{}



	template<std::derived_from<Event> EventType>
	EventReceiver<EventType>::~EventReceiver()
	{
		EventManager::get_instance().unregister_receiver(typeid(EventType), this);
	}



	template<std::derived_from<Event> EventType>
	void EventReceiver<EventType>::internal_handle_event(Event const& event)
	{
		on_event(static_cast<EventType const&>(event));
	}


	template<std::derived_from<Event> EventType>
	EventReceiverHandle<EventType>::EventReceiverHandle(std::function<void(EventType const&)> callback) :
		mCallback{std::move(callback)}
	{}



	template<std::derived_from<Event> EventType>
	void EventReceiverHandle<EventType>::on_event(EventType const& event)
	{
		mCallback(event);
	}
}
