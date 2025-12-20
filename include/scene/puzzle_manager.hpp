#pragma once

#include <cstddef>
#include <functional>

class PuzzleManager {
public:
  void checkCompletion() {
    clickCount += 1;
    if (clickCount == FINISH_COUNT) {
      onComplete(this);
    }
  }

  std::function<void(PuzzleManager *)> onComplete = [](PuzzleManager *) {};

  bool completed() { return clickCount == FINISH_COUNT; }

private:
  size_t clickCount;
  static constexpr size_t FINISH_COUNT = 3;
};
