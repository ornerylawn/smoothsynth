#include "synth.h"

#include <cmath>
#include <chrono> 

using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::microseconds;

const float kDriftCentsTable[] = {0, 18, 6, -12, -6, 12};
const float kLooseness = 0.3;

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
  for (int i = 0; i < voices_; ++i) {
    vcos_[i].set_drift_cents(kLooseness * kDriftCentsTable[i]);
    vcos_[i].set_cv_in(sequencer_.frequency_cv_outs(i));
    unisons_[i].set_mono_in(vcos_[i].mono_out());
    filters_[i].set_stereo_in(unisons_[i].stereo_out());

    adsrs_[i].set_trigger_in(sequencer_.trigger_outs(i));
    vcas_[i].set_stereo_in(filters_[i].stereo_out());
    vcas_[i].set_cv_in(adsrs_[i].cv_out());
    
    mixer_.set_stereo_in(i, vcas_[i].stereo_out());
  }
}

bool Synth::Rx() const { return sequencer_.Rx(); }

void Synth::ComputeAndStartTx(int frame_count) {
  CHECK(stereo_out_.capacity() >= frame_count*2);
  stereo_out_.set_size(frame_count * 2);
  float* stereo_out = stereo_out_.write_ptr();

  auto sequencer_start = high_resolution_clock::now();
  sequencer_.ComputeAndStartTx(frame_count);
  auto sequencer_stop = high_resolution_clock::now();
  auto seq_us = duration_cast<microseconds>(sequencer_stop - sequencer_start);

  // TODO: update parameters based on midi received by sequencer? (should respect original timing).

  microseconds adsr_us(0);
  microseconds vco_us(0);
  microseconds uni_us(0);
  microseconds filt_us(0);
  microseconds vca_us(0);

  for (int i = 0; i < voices_; ++i) {
    auto vco_start = high_resolution_clock::now();
    vcos_[i].ComputeAndStartTx(frame_count);
    auto vco_stop = high_resolution_clock::now();
    auto unison_start = high_resolution_clock::now();
    unisons_[i].ComputeAndStartTx(frame_count);
    auto unison_stop = high_resolution_clock::now();

    // TODO: cutoff should be controlled by cv, coming from an envelope.
    filters_[i].set_cutoff(sequencer_.cutoff());
    auto filter_start = high_resolution_clock::now();
    filters_[i].ComputeAndStartTx(frame_count);
    auto filter_stop = high_resolution_clock::now();

    auto adsr_start = high_resolution_clock::now();
    adsrs_[i].ComputeAndStartTx(frame_count);
    auto adsr_stop = high_resolution_clock::now();
    auto vca_start = high_resolution_clock::now();
    vcas_[i].ComputeAndStartTx(frame_count);
    auto vca_stop = high_resolution_clock::now();

    adsr_us += duration_cast<microseconds>(adsr_stop - adsr_start);
    vco_us += duration_cast<microseconds>(vco_stop - vco_start);
    uni_us += duration_cast<microseconds>(unison_stop - unison_start);
    filt_us += duration_cast<microseconds>(filter_stop - filter_start);
    vca_us += duration_cast<microseconds>(vca_stop - vca_start);
  }

  auto mixer_start = high_resolution_clock::now();
  mixer_.ComputeAndStartTx(frame_count);
  auto mixer_stop = high_resolution_clock::now();
  auto mix_us = duration_cast<microseconds>(mixer_stop - mixer_start);

  // Clip.
  auto clip_start = high_resolution_clock::now();
  const float* mixer_stereo_out = mixer_.stereo_out()->read_ptr();
  for (int i = 0; i < frame_count * 2; ++i) {
    double x = mixer_stereo_out[i];
    //stereo_out[i] = (x < -1.0f ? -1.0f : (x > 1.0f ? 1.0f : x));
    stereo_out[i] = std::tanh(x);
  }
  auto clip_stop = high_resolution_clock::now();
  auto clp_us = duration_cast<microseconds>(clip_stop - clip_start);
  stereo_out_.set_tx(true);

  //printf(
  //    "seq: %lld, adsr: %lld, vco: %lld, uni: %lld, vca: %lld, mix: %lld, clp: "
  //    "%lld\n",
  //    seq_us.count(), adsr_us.count(), vco_us.count(), uni_us.count(),
  //    vca_us.count(), mix_us.count(), clp_us.count());
}

void Synth::StopTx() {
  stereo_out_.set_tx(false);
  mixer_.StopTx();
  for (int i = 0; i < voices_; ++i) {
    vcas_[i].StopTx();
    adsrs_[i].StopTx();
    filters_[i].StopTx();
    unisons_[i].StopTx();
    vcos_[i].StopTx();
  }
  sequencer_.StopTx();
}