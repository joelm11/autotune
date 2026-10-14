#include <gtest/gtest.h>

#include <cmath>
#include <complex>
#include <vector>

#include "../fft_impl.h"

const float PI = 3.141592653589793238460;

// Helper function to compare two complex vectors
bool CompareComplexVectors(const std::vector<std::complex<float>>& a,
                           const std::vector<std::complex<float>>& b,
                           float tol = 1e-3) {
  if (a.size() != b.size()) {
    ADD_FAILURE() << "Vectors have different sizes: " << a.size() << " vs "
                  << b.size();
    return false;
  }
  for (size_t i = 0; i < a.size(); ++i) {
    if (std::abs(a[i] - b[i]) > tol) {
      ADD_FAILURE() << "Vectors differ at index " << i << ": " << a[i] << " vs "
                    << b[i];
      return false;
    }
  }
  return true;
}

bool CompareRealVectors(const std::vector<float>& a,
                        const std::vector<float>& b, float tol = 1e-5) {
  if (a.size() != b.size()) {
    ADD_FAILURE() << "Vectors have different sizes: " << a.size() << " vs "
                  << b.size();
    return false;
  }
  for (size_t i = 0; i < a.size(); ++i) {
    if (std::abs(a[i] - b[i]) > tol) {
      ADD_FAILURE() << "Vectors differ at index " << i << ": " << a[i] << " vs "
                    << b[i];
      return false;
    }
  }
  return true;
}

// Naive FFT function
void NaiveFFT(std::vector<std::complex<float>>& a) {
  const size_t N = a.size();
  if (N <= 1) return;

  // Divide
  std::vector<std::complex<float>> even(N / 2);
  std::vector<std::complex<float>> odd(N / 2);
  for (size_t i = 0; i < N / 2; ++i) {
    even[i] = a[i * 2];
    odd[i] = a[i * 2 + 1];
  }

  // Conquer
  NaiveFFT(even);
  NaiveFFT(odd);

  // Combine
  for (size_t k = 0; k < N / 2; ++k) {
    std::complex<float> t = std::polar(1.f, -2.f * PI * k / N) * odd[k];
    a[k] = even[k] + t;
    a[k + N / 2] = even[k] - t;
  }
}

class FFTTest : public ::testing::Test {
 protected:
  FFTTest() : fft(kFFTLen_) {}

  const size_t kFFTLen_ = 1024;
  FFTImpl fft;
};

// Test basic instantiation/deinitialization of the FFT implementation.
TEST_F(FFTTest, Instantiation) {}

// Test the forward FFT implementation against expected transform values
TEST_F(FFTTest, ForwardTransform) {
  std::vector<float> input(kFFTLen_);
  float sample_rate = 1000.0;  // Sample rate in Hz
  float frequency = 100.0;     // Frequency of the sine wave in Hz

  // Populate the input vector with a 100Hz sine wave
  for (size_t i = 0; i < kFFTLen_; ++i) {
    input[i] = std::sin(2 * PI * frequency * i / sample_rate);
  }

  std::vector<std::complex<float>> output(kFFTLen_);
  fft.Forward(input, output);

  // Compute the expected output using the naive FFT function
  std::vector<std::complex<float>> expected_output(input.begin(), input.end());
  NaiveFFT(expected_output);

  // Convert expected_output to std::complex<float>
  std::vector<std::complex<float>> expected_output_float(
      expected_output.begin(), expected_output.end());

  EXPECT_TRUE(CompareComplexVectors(output, expected_output_float));
}

// Test the inverse FFT implementation against expected transform values
TEST_F(FFTTest, InverseTransform) {
  std::vector<float> input(kFFTLen_);
  float sample_rate = 1000.0;  // Sample rate in Hz
  float frequency = 100.0;     // Frequency of the sine wave in Hz

  // Populate the input vector with a 100Hz sine wave
  for (size_t i = 0; i < kFFTLen_; ++i) {
    input[i] = std::sin(2 * PI * frequency * i / sample_rate);
  }

  std::vector<std::complex<float>> output(kFFTLen_);
  fft.Forward(input, output);

  std::vector<float> inverse_output(kFFTLen_);
  fft.Inverse(output, inverse_output);

  // Normalize the inverse output
  for (size_t i = 0; i < kFFTLen_; ++i) {
    inverse_output[i] /= kFFTLen_;
  }

  // Compare the inverse output with the original input
  EXPECT_TRUE(CompareRealVectors(inverse_output, input));
}