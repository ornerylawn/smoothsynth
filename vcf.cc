#include "vcf.h"

bool VCF::Rx() const {
  return stereo_in_ != nullptr && stereo_in_->tx();
}

void VCF::ComputeAndStartTx(int frame_count) {
  const float* stereo_in = stereo_in_->read_ptr();
  int sample_count = 2 * frame_count;
  stereo_out_.set_size(sample_count);
  float* stereo_out = stereo_out_.write_ptr();

  // TODO: cv input to control cutoff.

  //stereo_out[0] = stereo_in[0] + last_left_sample_;
  //stereo_out[1] = stereo_in[1] + last_right_sample_;
  //for (int i = 2; i < sample_count; ++i) {
  //  stereo_out[i] = stereo_in[i] + stereo_in[i - 2];
  //}
  //last_left_sample_ = stereo_in[sample_count - 2];
  //last_right_sample_ = stereo_in[sample_count - 1];


  // Andrew Simper's state-variable filter.
  //input  = input buffer;
  //output = output buffer;
  float fs     = sample_rate_;
  float fc     = cutoff_;
  float res    = 0.2;
  float drive  = 0.01;
  float freq   = 2.0*sin(PI*std::min(0.25f, fc/(fs*2)));  // the fs*2 is because it's double sampled
  float damp   = std::min(2.0*(1.0 - std::pow(res, 0.25)), std::min(2.0, 2.0/freq - freq*0.5));
  //notch  = notch output
  //low    = low pass output
  //high   = high pass output
  //band   = band pass output
  //peak   = peaking output = low - high
  
  for (int i = 0; i < frame_count; ++i) {
    float in = stereo_in[i*2];
    left_notch_ = in - damp*left_band_;
    left_low_   = left_low_ + freq*left_band_;
    left_high_  = left_notch_ - left_low_;
    left_band_  = freq*left_high_ + left_band_ - drive*left_band_*left_band_*left_band_;
    float out = 0.5*left_low_;
    left_notch_ = in - damp*left_band_;
    left_low_   = left_low_ + freq*left_band_;
    left_high_  = left_notch_ - left_low_;
    left_band_  = freq*left_high_ + left_band_ - drive*left_band_*left_band_*left_band_;
    out  += 0.5*left_low_;
    stereo_out[i*2] = out;
    stereo_out[i*2+1] = out;
  }

  stereo_out_.set_tx(true);
}

void VCF::StopTx() { stereo_out_.set_tx(false); }