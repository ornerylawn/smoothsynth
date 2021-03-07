#include "synth.h"

#include <cmath>

Synth::Synth(int sample_rate, int frames_per_chunk, int voices)
    : voices_(voices),
      sequencer_(sample_rate, frames_per_chunk, voices),
      adsrs_(voices, ADSR(sample_rate, frames_per_chunk)),
      vcos_(voices, VCO(sample_rate, frames_per_chunk, 0.0)),
      unisons_(voices, Unison(sample_rate, frames_per_chunk)),
      filters_(voices, VCF(sample_rate, frames_per_chunk)),
      vcas_(voices, VCA(sample_rate, frames_per_chunk)),
      mixer_(sample_rate, frames_per_chunk, voices),
      stereo_out_(frames_per_chunk * 2) {
  for (int i = 0; i < voices_; i++) {
    adsrs_[i].set_trigger_in(sequencer_.trigger_outs(i));

    vcos_[i].set_frequency_in(sequencer_.frequency_outs(i));
    unisons_[i].set_mono_in(vcos_[i].mono_out());
    filters_[i].set_stereo_in(unisons_[i].stereo_out());
    vcas_[i].set_stereo_in(filters_[i].stereo_out());
    vcas_[i].set_cv_in(adsrs_[i].cv_out());
    
    mixer_.set_stereo_in(i, vcas_[i].stereo_out());
  }
}

bool Synth::Rx() const { return sequencer_.Rx(); }

void Synth::ComputeAndStartTx(int frame_count) {
  stereo_out_.set_size(frame_count * 2);
  float* stereo_out = stereo_out_.write_ptr();

  sequencer_.ComputeAndStartTx(frame_count);

  for (int i = 0; i < voices_; ++i) {
    adsrs_[i].ComputeAndStartTx(frame_count);
    vcos_[i].ComputeAndStartTx(frame_count);
    unisons_[i].ComputeAndStartTx(frame_count);
    filters_[i].ComputeAndStartTx(frame_count);
    vcas_[i].ComputeAndStartTx(frame_count);
  }

  mixer_.ComputeAndStartTx(frame_count);

  // Clip.
  const float* mixer_stereo_out = mixer_.stereo_out()->read_ptr();
  for (int i = 0; i < frame_count * 2; ++i) {
    stereo_out[i] = std::max(-1.0f, std::min(1.0f, mixer_stereo_out[i]));
  }
  stereo_out_.set_tx(true);
}

void Synth::StopTx() {
  stereo_out_.set_tx(false);
  mixer_.StopTx();
  for (int i = 0; i < voices_; ++i) {
    vcas_[i].StopTx();
    filters_[i].StopTx();
    unisons_[i].StopTx();
    vcos_[i].StopTx();
    adsrs_[i].StopTx();
  }
  sequencer_.StopTx();
}