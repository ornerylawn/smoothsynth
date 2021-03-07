#include "mixer.h"

bool Mixer::Rx() const {
  for (int i = 0; i < stereo_ins_.size(); ++i) {
    if (stereo_ins_[i] == nullptr || !stereo_ins_[i]->tx()) {
      return false;
    }
  }
  return true;
}

void Mixer::ComputeAndStartTx(int frame_count) {
  int sample_count = 2*frame_count;
  CHECK(stereo_out_.capacity() >= sample_count);
  stereo_out_.set_size(sample_count);
  float* stereo_out = stereo_out_.write_ptr();
  
  for (int i = 0; i < sample_count; ++i) {
    stereo_out[i] = 0.0;
  }
  float amp = 1.0 / std::sqrt(stereo_ins_.size());
  for (int i = 0; i < stereo_ins_.size(); ++i) {
    CHECK(stereo_ins_[i] != nullptr && stereo_ins_[i]->tx());
    CHECK(stereo_ins_[i]->size() == frame_count*2);
    const float* stereo_in = stereo_ins_[i]->read_ptr();
    for (int j = 0; j < sample_count; ++j) {
      stereo_out[j] += amp * stereo_in[j];
    }
  }
  stereo_out_.set_tx(true);
}

void Mixer::StopTx() { stereo_out_.set_tx(false); }
