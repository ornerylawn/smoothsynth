#include "convolution_reverb.h"

#include <cmath>

ConvolutionReverb::ConvolutionReverb(int sample_rate, int frames_per_chunk)
  : decay_(Duration(10)*Millisecond),
    duration_per_frame_(DurationPerFrame(sample_rate)),
    impulse_response_(NextPowerOf2(decay_/duration_per_frame_)),
    input_ring_(2*impulse_response_.size()),
    stereo_out_(2*frames_per_chunk),
    w_(0) {
  wrap_mask_ = input_ring_.size()-1;
  for (int i = 0; i < impulse_response_.size(); i++) {
    Duration t = Duration(i)*duration_per_frame_;
    float response = std::exp(-5*(t / decay_));
    impulse_response_[i] = response / 5.0;  // compensate for volume increase
  }
  for (int i = 0; i < input_ring_.size(); i++) {
    input_ring_[i] = 0.0;
  }
}

void ConvolutionReverb::Compute(int frame_count) {
  const float* stereo_in = stereo_in_->read_ptr();
  stereo_out_.set_size(frame_count*2);
  float* stereo_out = stereo_out_.write_ptr();

  // TODO: really need to do this with FFT in order to have long
  // reverbs (with very short reverbs it sounds like low pass
  // filter?).

  for (int i = 0; i < 2*frame_count; i += 2) {
    w_ = (w_+2) & wrap_mask_;
    input_ring_[w_] = stereo_in[i];
    input_ring_[w_+1] = stereo_in[i+1];
    
    float left = 0.0;
    float right = 0.0;
    for (int past = 0; past < impulse_response_.size(); past++) {
      float ir = impulse_response_[past];
      int r = (w_-2*past) & wrap_mask_;
      left += input_ring_[r]*ir;
      right += input_ring_[r+1]*ir;
    }
    
    stereo_out[i] = left;
    stereo_out[i+1] = right;
  }

  // for (int i = 0; i < 2*frame_count; i++) {
  //   stereo_out[i] = stereo_in[i];
  // }
}
