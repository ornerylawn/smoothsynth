#include "unison.h"

bool Unison::Rx() const override {
  return mono_in_ != nullptr && mono_in_->tx();
}

void Unison::ComputeAndStartTx(int frame_count) {
  const float* mono_in = mono_in_->read_ptr();

  stereo_out_.set_size(frame_count * 2);
  float* stereo_out = stereo_out_.write_ptr();

  for (int i = 0; i < frame_count; ++i) {
    float sample = mono_in[i];
    stereo_out[2 * i] = sample;
    stereo_out[2 * i + 1] = sample;
  }
  stereo_out_.set_tx(true);
}

void Unison::StopTx() override { stereo_out_.set_tx(false); }