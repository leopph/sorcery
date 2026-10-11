#pragma once

#include <concepts>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <utility>


namespace sorcery {
template<typename ObjectType, bool Shared = false>
class Mutex;


template<typename ObjectType, typename LockType, bool Shared>
class MutexGuard {
public:
  [[nodiscard]] auto operator*() -> ObjectType& requires (!Shared) {
    return *object_;
  }


  [[nodiscard]] auto operator*() const -> ObjectType const& {
    return *object_;
  }


  [[nodiscard]] auto operator->() -> ObjectType* requires (!Shared) {
    return object_;
  }


  [[nodiscard]] auto operator->() const -> ObjectType const* {
    return object_;
  }

private:
  MutexGuard(ObjectType& object, LockType lock) :
    object_{&object},
    lock_{std::move(lock)} {}


  ObjectType* object_;
  LockType lock_;

  friend Mutex<ObjectType>;
  friend Mutex<ObjectType, true>;
};


template<typename ObjectType, bool Shared>
class Mutex {
  using MutexType = std::conditional_t<Shared, std::shared_mutex, std::mutex>;

public:
  template<typename... Args>
  explicit(sizeof...(Args) == 1) Mutex(Args&&... args) :
    object_{std::forward<Args>(args)...} {}


  auto Lock() -> MutexGuard<ObjectType, std::unique_lock<MutexType>, false> {
    return MutexGuard<ObjectType, std::unique_lock<MutexType>, false>{object_, std::unique_lock{mutex_}};
  }


  auto LockShared() -> MutexGuard<ObjectType, std::shared_lock<MutexType>, true> requires(Shared) {
    return MutexGuard<ObjectType, std::shared_lock<MutexType>, true>{object_, std::shared_lock{mutex_}};
  }


  auto TryLock() -> std::optional<MutexGuard<ObjectType, std::unique_lock<MutexType>, false>> {
    if (std::unique_lock lock{mutex_, std::try_to_lock}; lock.owns_lock()) {
      return MutexGuard<ObjectType, std::unique_lock<MutexType>, false>{object_, std::move(lock)};
    }

    return std::nullopt;
  }


  auto TryLockShared() -> std::optional<MutexGuard<ObjectType, std::shared_lock<MutexType>, true>> requires(Shared) {
    if (std::shared_lock lock{mutex_, std::try_to_lock}; lock.owns_lock()) {
      return MutexGuard<ObjectType, std::shared_lock<MutexType>, true>{object_, std::move(lock)};
    }

    return std::nullopt;
  }

private:
  ObjectType object_;
  MutexType mutex_;
};
}
