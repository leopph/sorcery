#pragma once

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>


namespace sorcery {
namespace details {
using EventListenerReturnType = void;

template<typename... EventParams>
using EventListenerEntry = std::function<void(EventParams...)>;
}


template<typename T, typename... EventParams>concept EventListener = std::is_invocable_r_v<
  details::EventListenerReturnType, T, EventParams...>;


template<typename... EventParams>
using EventListenerHandle = std::function<details::EventListenerReturnType(EventParams...)> const*;


template<class... EventParams>
class Event {
public:
  using HandleType = EventListenerHandle<EventParams...>;

private:
  using EntryType = details::EventListenerEntry<EventParams...>;
  std::vector<std::unique_ptr<EntryType>> listeners_;

public:
  auto invoke(EventParams&&... params) const -> void {
    for (auto const& listener : listeners_) {
      listener->operator()(std::forward<EventParams>(params)...);
    }
  }


  template<EventListener<EventParams...> ListenerType>
  auto add_listener(ListenerType&& listener) -> HandleType {
    return listeners_.emplace_back(std::make_unique<EntryType>(std::forward<ListenerType>(listener))).get();
  }


  auto remove_listener(HandleType const listener) -> void {
    std::erase_if(listeners_, [listener](std::unique_ptr<EntryType> const& entry) {
      return entry.get() == listener;
    });
  }
};


template<class... EventParams>
class GuardedEventReference {
public:
  using EventType = Event<EventParams...>;
  using HandleType = typename EventType::HandleType;


  explicit GuardedEventReference(Event<EventParams...>& event) :
    event_{&event} {}


  template<EventListener<EventParams...> ListenerType>
  auto add_listener(ListenerType&& listener) -> HandleType {
    return event_->add_listener(std::forward<ListenerType>(listener));
  }


  auto remove_listener(HandleType const listener) -> void {
    return event_->remove_listener(listener);
  }

private:
  EventType* event_;
};
}
