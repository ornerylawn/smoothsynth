#include "vco.h"

#include <cmath>

namespace {

const int sample_count = 2048;
const int table_count = 10;
const float lo_f = 13.75;    // lowest A
const float hi_f = 14080.0;  // highest A
const float max_power_distance = std::log2(hi_f) - std::log2(lo_f);

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
      seconds_per_frame_(SecondsPerFrame(sample_rate)),
      radians_(0.0f),
      drift_radians_(0.0f),
      drift_f_(0.6f),
      drift_amp_(0.3f),
      drift_offset_(drift_offset),
      wave_table_(table_count * sample_count) {
  for (int i = 0; i < table_count; i++) {
    int harmonics = HarmonicsForTable(i);
    float* buf = wave_table_.buf() + (i * sample_count);
    InitSawTable(buf, sample_count, harmonics);
  }

  // float power_step = (power_distance-1.0) / (wave_table_count-1);
  // for (int i = 0; i <= wave_table_count+1; i++) {
  //   float f = lo_f*std::pow(2.0, 1.0 + i*power_step);
  //   int harmonics = int((wave_table_size * 0.5 * 0.99) / f);
  //   int index_offset = i*(wave_table_size+2);

  //   float* buf = wave_table_+index_offset;
  //   InitSawTable(buf, wave_table_size+1, harmonics);
  // }
}

bool VCO::Rx() const { return frequency_in_ != nullptr && frequency_in_->tx(); }

void VCO::ComputeAndStartTx(int frame_count) {
  const float* frequency_in = frequency_in_->read_ptr();

  mono_out_.set_size(frame_count);
  float* mono_out = mono_out_.write_ptr();

  for (int i = 0; i < frame_count; i++) {
    // float drift = std::sin(drift_radians_);
    float f = frequency_in[i];
    // float f = frequency_in[i] + drift_amp_*drift + drift_offset_;

    float power_distance = std::log2(f) - std::log2(lo_f);
    float power_fraction = std::min(1.0f, power_distance / max_power_distance);
    float continuous_index = power_fraction * (table_count - 2);
    int left_index = int(continuous_index);
    float lerp_amount = continuous_index - left_index;

    float* left_buf = wave_table_.buf() + left_index * sample_count;
    float left = ReadLinear(left_buf, sample_count, radians_);
    float* right_buf = wave_table_.buf() + (left_index + 1) * sample_count;
    float right = ReadLinear(right_buf, sample_count, radians_);

    mono_out[i] = LinearInterpolation(left, right, lerp_amount);

    float dr = RadiansPerFrame(f, seconds_per_frame_);
    radians_ = WrapRadians(radians_ + dr);
    // float drift_dr = RadiansPerFrame(drift_f_, seconds_per_frame_);
    // drift_radians_ = WrapRadians(drift_radians_ + drift_dr);
  }

  mono_out_.set_tx(true);
}

void VCO::StopTx() override { mono_out_.set_tx(false); }