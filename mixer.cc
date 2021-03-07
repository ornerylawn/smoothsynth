#include "mixer.h"

bool Mixer::Rx() const override {
    for (int i = 0; i < stereo_ins_.size(); i++) {
      if (stereo_ins_[i] == nullptr || !stereo_ins_[i]->tx()) {
        return false;
      }
    }
    return true;
  }

void Mixer::ComputeAndStartTx(int frame_count) {
  int sample_count = 2*frame_count;
  stereo_out_.set_size(sample_count);
  float* stereo_out = stereo_out_.write_ptr();
  
  for (int i = 0; i < sample_count; ++i) {
    stereo_out[i] = 0.0;
  }
  for (int i = 0; i < stereo_ins_.size(); ++i) {
    const float* stereo_in = stereo_ins_[i]->read_ptr();
    for (int j = 0; j < sample_count; ++j) {
      stereo_out[j] += stereo_in[j];
    }
  }
  stereo_out_.set_tx(true);
}

void Mixer::StopTx() override { stereo_out_.set_tx(false); }
