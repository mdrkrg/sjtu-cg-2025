#pragma once

#include <functional>
#include <unordered_map>

class TrapManager {
public:
  using callback_id = void *;

  void trigger() {
    if (triggered()) {
      return;
    }
    _triggered = true;
    notifyTriggered();
  }

  void registerTriggerCallback(callback_id id,
                               std::function<void()> &&callback) {
    triggerCallbacks[id] = std::move(callback);
  }

  void unregisterTriggerCallback(callback_id id) { triggerCallbacks.erase(id); }

  bool triggered() const { return _triggered; }

private:
  bool _triggered;
  std::unordered_map<callback_id, std::function<void()>> triggerCallbacks;

  /// Notify all registered callbacks when triggered
  void notifyTriggered() {
    for (const auto &[_, callback] : triggerCallbacks) {
      if (callback)
        callback();
    }
  }
};
