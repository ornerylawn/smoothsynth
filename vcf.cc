#include "vcf.h"

bool VCF::Rx() const override {
  return stereo_in_ != nullptr && stereo_in_->tx();
}

void VCF::Compute(int frame_count) {
  const float* stereo_in = stereo_in_->read_ptr();
  int sample_count = 2 * frame_count;
  stereo_out_.set_size(sample_count);
  float* stereo_out = stereo_out_.write_ptr();

  // TODO: cv input to control cutoff.

  stereo_out[0] = stereo_in[0] + last_left_sample_;
  stereo_out[1] = stereo_in[1] + last_right_sample_;
  for (int i = 2; i < sample_count; ++i) {
    stereo_out[i] = stereo_in[i] + stereo_in[i - 2];
  }
  last_left_sample_ = stereo_in[sample_count - 2];
  last_right_sample_ = stereo_in[sample_count - 1];
  stereo_out_.set_tx(true);
}

void VCF::StartTx() override { stereo_out_.set_tx(true); }