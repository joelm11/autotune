#ifndef SMOOTHING_H
#define SMOOTHING_H

#include <cmath>
#include <optional>

namespace smoothing {

// One-pole coefficient per update; larger moves faster.
constexpr float kDefaultCoefficient = 0.8f;

// Updates at or below this delta are treated as settled and not reported.
constexpr float kDefaultMinDelta = 1e-6f;

// Stateful value smoother, swappable behind a common call site.
class Smoother {
 public:
  virtual ~Smoother() = default;

  // Returns the new value, or nullopt if the update is a no-op.
  virtual std::optional<float> Smooth(float target) = 0;
};

// Exponential smoother: value += coefficient * (target - value).
class OnePoleSmoother final : public Smoother {
 public:
  OnePoleSmoother(const float coefficient, const float initial,
                  const float min_delta = kDefaultMinDelta)
      : coefficient_(coefficient), value_(initial), min_delta_(min_delta) {}

  std::optional<float> Smooth(const float target) override {
    const float next = value_ + coefficient_ * (target - value_);
    if (std::fabs(next - value_) <= min_delta_) {
      return std::nullopt;
    }
    value_ = next;
    return value_;
  }

  float value() const { return value_; }

 private:
  const float coefficient_;
  float value_;
  const float min_delta_;
};

}  // namespace smoothing
#endif  // SMOOTHING_H
