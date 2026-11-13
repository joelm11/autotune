#include "../smoothing.hh"

#include <gtest/gtest.h>

TEST(OnePoleSmootherTest, StepsByCoefficientTowardTarget) {
  smoothing::OnePoleSmoother smoother(0.5f, 0.f);

  const auto first = smoother.Smooth(1.f);
  ASSERT_TRUE(first.has_value());
  EXPECT_FLOAT_EQ(*first, 0.5f);

  const auto second = smoother.Smooth(1.f);
  ASSERT_TRUE(second.has_value());
  EXPECT_FLOAT_EQ(*second, 0.75f);
}

TEST(OnePoleSmootherTest, ConvergesToTarget) {
  smoothing::OnePoleSmoother smoother(0.5f, 0.f);
  for (int i = 0; i < 100; ++i) {
    smoother.Smooth(2.f);
  }
  EXPECT_NEAR(smoother.value(), 2.f, 1e-4f);
}

TEST(OnePoleSmootherTest, NoUpdateWhenAlreadyAtTarget) {
  smoothing::OnePoleSmoother smoother(0.5f, 1.f);
  EXPECT_FALSE(smoother.Smooth(1.f).has_value());
}

TEST(OnePoleSmootherTest, IgnoresSubThresholdChanges) {
  // A min_delta of 0.1 makes a step of at most 0.05 a no-op.
  smoothing::OnePoleSmoother smoother(0.5f, 0.f, 0.1f);
  EXPECT_FALSE(smoother.Smooth(0.05f).has_value());
  EXPECT_FLOAT_EQ(smoother.value(), 0.f);

  // A large enough move is still reported.
  const auto moved = smoother.Smooth(1.f);
  ASSERT_TRUE(moved.has_value());
  EXPECT_FLOAT_EQ(*moved, 0.5f);
}

TEST(OnePoleSmootherTest, SmoothsInTheNegativeDirection) {
  smoothing::OnePoleSmoother smoother(0.25f, 10.f);
  const auto next = smoother.Smooth(0.f);
  ASSERT_TRUE(next.has_value());
  EXPECT_FLOAT_EQ(*next, 7.5f);
}
