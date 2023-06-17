#pragma once

#include <memory>
#include <utility>
#include <vector>


namespace leopph {
template<class... EventParams>
class Event {
public:
  using FreeHandlerType = void(*)(EventParams...);

  template<class T>
  using MemberHandlerType = void(*)(T*, EventParams...);

private:
  class EventHandler {
  public:
    virtual auto invoke(EventParams&&... params) const -> void = 0;
    virtual auto equals(void* explicitThis, void* handler) const -> bool = 0;
    virtual ~EventHandler() = default;
  };


  class FreeEventHandler : public EventHandler {
  private:
    FreeHandlerType mHandler;

  public:
    explicit FreeEventHandler(FreeHandlerType const handler) :
      mHandler{ handler } {}


    auto invoke(EventParams&&... params) const -> void override {
      mHandler(std::forward<EventParams>(params)...);
    }


    auto equals([[maybe_unused]] void* const explicitThis, void* const handler) const -> bool override {
      return reinterpret_cast<void*>(mHandler) == handler;
    }
  };


  template<class T>
  class MemberEventHandler : public EventHandler {
  private:
    MemberHandlerType<T> mHandler;
    T* mHandlerExplicitThis;

  public:
    MemberEventHandler(T* const handlerExplicitThis, MemberHandlerType<T> handler) :
      mHandler{ handler },
      mHandlerExplicitThis{ handlerExplicitThis } {}


    auto invoke(EventParams&&... params) const -> void override {
      mHandler(mHandlerExplicitThis, std::forward<EventParams>(params)...);
    }


    auto equals(void* const explicitThis, void* const handler) const -> bool override {
      return mHandlerExplicitThis == reinterpret_cast<T*>(explicitThis) && mHandler == reinterpret_cast<MemberHandlerType<T>>(handler);
    }
  };


  std::vector<std::unique_ptr<EventHandler>> mHandlers;

public:
  auto invoke(EventParams&&... params) const -> void {
    for (auto const& handler : mHandlers) {
      handler->invoke(std::forward<EventParams>(params)...);
    }
  }


  auto add_handler(FreeHandlerType const handler) -> void {
    mHandlers.emplace_back(std::make_unique<FreeEventHandler>(handler));
  }


  auto remove_handler(FreeHandlerType const handler) -> void {
    std::erase_if(mHandlers, [handler](std::unique_ptr<EventHandler> const& eventHandler) {
      return eventHandler->equals(nullptr, handler);
    });
  }


  template<class T>
  auto add_handler(T* const explicitThis, MemberHandlerType<T> const handler) -> void {
    mHandlers.emplace_back(std::make_unique<MemberEventHandler<T>>(explicitThis, handler));
  }


  template<class T>
  auto remove_handler(T* const explicitThis, MemberHandlerType<T> const handler) -> void {
    std::erase_if(mHandlers, [explicitThis, handler](std::unique_ptr<EventHandler> const& eventHandler) {
      return eventHandler->equals(reinterpret_cast<void*>(explicitThis), reinterpret_cast<void*>(handler));
    });
  }


  auto operator()(EventParams&&... params) const -> void {
    invoke(std::forward<EventParams>(params)...);
  }


  auto operator+=(FreeHandlerType const handler) -> void {
    add_handler(handler);
  }


  auto operator-=(FreeHandlerType const handler) -> void {
    remove_handler(handler);
  }
};


template<class... EventParams>
class GuardedEventReference {
private:
  Event<EventParams...>& mEvent;

public:
  using FreeHandlerType = typename Event<EventParams...>::FreeHandlerType;

  template<class T>
  using MemberHandlerType = typename Event<EventParams...>::template MemberHandlerType<T>;


  GuardedEventReference(Event<EventParams...>& event) :
    mEvent{ event } {}


  auto add_handler(FreeHandlerType const handler) -> void {
    mEvent.add_handler(handler);
  }


  auto remove_handler(FreeHandlerType const handler) -> void {
    mEvent.remove_handler(handler);
  }


  template<class T>
  auto add_handler(T* explicitThis, MemberHandlerType<T> const handler) -> void {
    mEvent.add_handler(explicitThis, handler);
  }


  template<class T>
  auto remove_handler(T* explicitThis, MemberHandlerType<T> handler) -> void {
    mEvent.remove_handler(explicitThis, handler);
  }


  auto operator+=(FreeHandlerType const handler) -> void {
    mEvent.add_handler(handler);
  }


  auto operator-=(FreeHandlerType handler) -> void {
    mEvent.remove_handler(handler);
  }
};
}
