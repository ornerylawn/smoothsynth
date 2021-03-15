#include "vco.h"

#include <cmath>

namespace {

// The first table will have harmonics for the range [13.75, 27.5), so it will
// have 20000/27.5 harmonics. The next table will play an octave higher [27.5,
// 55), so it will have 20000/55 harmonics. And so on. Since the first table has
// 727 harmonics, we need a table size of at least 727*2 to satisfy Nyquist.
constexpr float kLowestA = 13.75;
constexpr float kMaxHarmonicFrequency = 20000;
constexpr int kFirstTableHarmonics = kMaxHarmonicFrequency / (kLowestA * 2);
constexpr int kTableSize = NextPowerOf2(kFirstTableHarmonics * 2 + 1);
constexpr int kTableCount = std::log2(20000 - kLowestA * 2);
constexpr float kCentsPerCV = (1.0 / 12.0) / 100.0;

//const float lo_f = 13.75;    // lowest A
//const float log_lo_f = std::log2(lo_f);
//const float hi_f = 14080.0;  // highest A
//const float log_hi_f = std::log2(hi_f);
//const float max_power_distance = log_hi_f - log_lo_f;

int HarmonicsForTable(int i) {
  float max_table_freq = kLowestA * (1<<(i+1));  // 27.5,...,440,...,28160
  return static_cast<int>(kMaxHarmonicFrequency / max_table_freq);
}

float SampleSaw(float radians, int harmonics) {
  float sum = 0.0;
  for (int k = 1; k <= harmonics; ++k) {
    sum += std::pow(-1, k) * std::sin(radians * k) / k;
  }
  return (2.0 / PI) * sum;
}

void FillSawTable(int samples, int harmonics, float* buf) {
  // Last sample should be radians=2pi. The last usable index is actually
  // samples-2, but we'll lerp with samples-1, so it should have the same sample
  // as radians=0.
  for (int i = 0; i < samples; ++i) {
    float rotations = i / static_cast<float>(samples-1);
    float radians = TWO_PI * rotations;
    buf[i] = SampleSaw(radians, harmonics);
  }
}

}  // namespace

VCO::VCO(int sample_rate, int frames_per_chunk, float drift_cents)
    : drift_cv_(drift_cents * kCentsPerCV),
      seconds_per_frame_(SecondsPerFrame(sample_rate)),
      cv_in_(nullptr),
      mono_out_(frames_per_chunk),
      wave_table_(kTableCount * kTableSize),
      radial_fraction_(0.0f) {
  std::cout << "kTableSize: " << kTableSize << "\n"
            << "kTableCount: " << kTableCount << "\n";
  for (int i = 0; i < kTableCount; i++) {
    int harmonics = HarmonicsForTable(i);
    float* buf = wave_table_.buf() + (i * kTableSize);
    FillSawTable(kTableSize, harmonics, buf);

    float f_lo = kLowestA * (1<<i);
    float f_hi = kLowestA * (1<<(i+1));
    if (i == 0) {
      f_lo = 0.0f;
    }
    if (i == kTableCount-1) {
      f_hi = 20000.0f;
    }
    std::cout << "wave table " << i << ": " << harmonics << " harmonics "
              << ", [" << f_lo << ", " << f_hi << ")"
              << ", first sample = " << buf[0]
              << ", last sample = " << buf[kTableSize - 1] << "\n";
  }
}

bool VCO::Rx() const { return cv_in_ != nullptr && cv_in_->tx(); }

void VCO::ComputeAndStartTx(int frame_count) {
  const float* cv_in = cv_in_->read_ptr();
  mono_out_.set_size(frame_count);
  float* mono_out = mono_out_.write_ptr();

  for (int i = 0; i < frame_count; ++i) {
    float frequency = kLowestA*std::pow(2, cv_in_[i] + drift_cv_);
    float power_distance = std::log2(frequency) - std::log2(kLowestA * 2);
    int table_index =
        (power_distance < 0.0f ? 0
                               : (power_distance >= kTableCount
                                      ? kTableCount - 1
                                      : static_cast<int>(power_distance)));

    float* buf = wave_table_.buf() + table_index * kTableSize;
    float continuous_index = radial_fraction_ * (kTableSize - 1);
    int left_index = static_cast<int>(continuous_index);
    float lerp_amount = continuous_index - left_index;
    mono_out[i] =
        LinearInterpolation(buf[left_index], buf[left_index + 1], lerp_amount);

    radial_fraction_ += frequency * seconds_per_frame_;
    if (radial_fraction_ >= 1.0) {
      radial_fraction_ -= 1.0;
    }
  }

  mono_out_.set_tx(true);
}

void VCO::StopTx() { mono_out_.set_tx(false); }