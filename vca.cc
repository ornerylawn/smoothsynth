#include "vca.h"

bool VCA::Rx() const {
  return stereo_in_ != nullptr && stereo_in_.tx() && cv_in_ != nullptr &&
         cv_in_.tx();
}

void VCA::ComputeAndStartTx(int frame_count) {
  const float* stereo_in = stereo_in_.read_ptr();
  const float* cv_in = cv_in_.read_ptr();
  stereo_out_.set_size(frame_count * 2);
  float* stereo_out = stereo_out_.write_ptr();
  for (int i = 0; i < frame_count * 2; ++i) {
    stereo_out[i] = cv_in[i] * stereo_in[i];
  }
  stereo_out_.set_tx(true);
}

void VCA::StopTx() { stereo_out_.set_tx(false); }