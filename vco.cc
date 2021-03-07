#include "vco.h"

#include <cmath>

namespace {

const int sample_count = 2048;
const int table_count = 10;
const float lo_f = 13.75;    // lowest A
const float log_lo_f = std::log2(lo_f);
const float hi_f = 14080.0;  // highest A
const float log_hi_f = std::log2(hi_f);
const float max_power_distance = log_hi_f - log_lo_f;

int HarmonicsForTable(int i) {
  float table_fraction = i / static_cast<float>(table_count - 2);
  return LinearInterpolation(sample_count / 2.0f, 1.0f,
                             std::min(1.0f, table_fraction));
}

float SampleSine(float radians) { return std::sin(radians); }

float SampleSaw(float radians, int harmonics) {
  float sum = 0.0;
  for (int k = 1; k <= harmonics; k++) {
    sum += std::pow(-1, k) * SampleSine(radians * k) / k;
  }
  return (2.0 / PI) * sum;
}

void InitSawTable(float* buf, int samples, int harmonics) {
  for (int i = 0; i < samples; i++) {
    float radians = TWO_PI * i / static_cast<float>(samples - 2);
    buf[i] = SampleSaw(radians, harmonics);
  }
}

float ReadLinear(float* buf, int samples, float radians) {
  float radial_fraction = radians / TWO_PI;
  float continuous_index = radial_fraction * (samples - 2);
  int left_index = int(continuous_index);
  float lerp_amount = continuous_index - left_index;
  return LinearInterpolation(buf[left_index], buf[left_index + 1], lerp_amount);
}

}  // namespace

VCO::VCO(int sample_rate, int frames_per_chunk, float drift_offset)
    : frequency_in_(nullptr),
      mono_out_(frames_per_chunk),
      frequency_to_radians_(SecondsPerFrame(sample_rate) * TWO_PI),
      radians_(0.0f),
      wave_table_(table_count * sample_count) {
  for (int i = 0; i < table_count; i++) {
    int harmonics = HarmonicsForTable(i);
    //printf("harmonics: %d (table %d)\n", harmonics, i);
    float* buf = wave_table_.buf() + (i * sample_count);
    InitSawTable(buf, sample_count, harmonics);
  }
}

bool VCO::Rx() const { return frequency_in_ != nullptr && frequency_in_->tx(); }

void VCO::ComputeAndStartTx(int frame_count) {
  CHECK(frequency_in_ != nullptr && frequency_in_->tx());
  CHECK(frequency_in_->size() == frame_count);
  const float* frequency_in = frequency_in_->read_ptr();

  CHECK(mono_out_.capacity() >= frame_count);
  mono_out_.set_size(frame_count);
  float* mono_out = mono_out_.write_ptr();

  for (int i = 0; i < frame_count; i++) {
    float f = frequency_in[i];

    float power_distance = std::log2(f) - log_lo_f;
    float power_fraction = (power_distance > max_power_distance
                                ? 1.0f
                                : power_distance / max_power_distance);
    float continuous_index = power_fraction * (table_count - 2);
    int left_index = int(continuous_index);
    float lerp_amount = continuous_index - left_index;

    float* left_buf = wave_table_.buf() + left_index * sample_count;
    float left = ReadLinear(left_buf, sample_count, radians_);
    //float* right_buf = wave_table_.buf() + (left_index + 1) * sample_count;
    //float right = ReadLinear(right_buf, sample_count, radians_);

    mono_out[i] = left; //LinearInterpolation(left, right, lerp_amount);

    radians_ += f * frequency_to_radians_;
    if (radians_ > TWO_PI) {
      radians_ -= TWO_PI;
    }
  }

  mono_out_.set_tx(true);
}

void VCO::StopTx() { mono_out_.set_tx(false); }