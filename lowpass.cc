#include "lowpass.h"

void LowPass::Compute(int frame_count) {
  const float* stereo_in = stereo_in_->read_ptr();

  stereo_out_.set_size(2*frame_count);
  float* stereo_out = stereo_out_.write_ptr();

  // Filter.
  // mono_out[0] = mono_in[0] + last_sample_;
  // for (int i = 1; i < frame_count; i++) {
  //   mono_out[i] = mono_in[i] + mono_in[i-1];
  // }
  // last_sample_ = mono_in[frame_count-1];

  stereo_out[0] = stereo_in[0] + last_left_sample_;
  stereo_out[1] = stereo_in[1] + last_left_sample_;
  for (int i = 2; i < 2*frame_count; i += 2) {
    stereo_out[i] = stereo_in[i] + stereo_in[i-2];
    stereo_out[i+1] = stereo_in[i+1] + stereo_in[i-1];
  }
  last_left_sample_ = stereo_in[2*frame_count-2];
  last_right_sample_ = stereo_in[2*frame_count-1];

  // No filter.
  //for (int i = 0; i < 2*frame_count; i++) {
  //  stereo_out[i] = stereo_in[i];
  //}
}
