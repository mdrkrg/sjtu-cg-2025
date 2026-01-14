#pragma once

#include <cstddef>
#include <functional>
#include <unordered_map>

class PuzzleManager {
public:
  using callback_id = void *;

  void checkCompletion() {
    clickCount += 1;
    if (clickCount == FINISH_COUNT) {
      notifyPuzzleSolved();
      onComplete(this);
    }
  }

  void releaseOne() { clickCount -= 1; }

  void registerPuzzleCallback(callback_id id,
                              std::function<void()> &&callback) {
    puzzleCallbacks[id] = std::move(callback);
  }

  void unregisterPuzzleCallback(callback_id id) { puzzleCallbacks.erase(id); }

  std::function<void(PuzzleManager *)> onComplete{[](PuzzleManager *) {}};

  bool completed() const { return clickCount == FINISH_COUNT; }

private:
  size_t clickCount{0};
  static constexpr size_t FINISH_COUNT{3};
  std::unordered_map<callback_id, std::function<void()>> puzzleCallbacks;

  /// Notify all registered callbacks when the puzzle is solved
  void notifyPuzzleSolved() {
    for (const auto &[_, callback] : puzzleCallbacks) {
      if (callback)
        callback();
    }
  }
};
