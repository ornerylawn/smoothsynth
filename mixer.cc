#include "mixer.h"

void Mixer::Compute(int frame_count) {
  stereo_out_.set_size(2*frame_count);
  float* stereo_out = stereo_out_.write_ptr();
  for (int i = 0; i < 2*frame_count; i++) {
    stereo_out[i] = 0.0;
  }

  for (int i = 0; i < stereo_ins_.size(); i++) {
    const float* stereo_in = stereo_ins_[i]->read_ptr();
    for (int j = 0; j < 2*frame_count; j++) {
      stereo_out[j] += stereo_in[j];
    }
  }
}
