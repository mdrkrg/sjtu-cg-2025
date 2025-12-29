#pragma once

#include <random>
namespace math {
inline static std::random_device _rd{};
inline static std::mt19937 _gen{_rd()};
inline static std::uniform_real_distribution<float> _uniformDist{0.0f, 1.0f};

inline float uniformDist() { return _uniformDist(_gen); }

} // namespace math
