#include "synth.h"

#include <cmath>

Synth::Synth(int sample_rate, int frames_per_chunk, int voices)
	: voices_(voices),
		sequencer_(sample_rate, frames_per_chunk, voices),
		vcos_(voices, VCO(sample_rate, frames_per_chunk, 0.0)),
		filters_(voices, LowPass(sample_rate, frames_per_chunk)),
		mono_to_stereos_(voices, MonoToStereo(sample_rate, frames_per_chunk)),
		adsrs_(voices, ADSR(sample_rate, frames_per_chunk)),
    mixer_(sample_rate, frames_per_chunk, voices),
		stereo_out_(frames_per_chunk*2) {
	for (int i = 0; i < voices_; i++) {
		vcos_[i].set_frequency_in(sequencer_.frequency_outs(i));
    // TODO: unison (copy, detune, spread).
		mono_to_stereos_[i].set_mono_in(vcos_[i].mono_out());
		filters_[i].set_stereo_in(mono_to_stereos_[i].stereo_out());
		adsrs_[i].set_stereo_in(filters_[i].stereo_out());
		adsrs_[i].set_trigger_in(sequencer_.trigger_outs(i));
    mixer_.set_stereo_in(i, adsrs_[i].stereo_out());
	}
}

void Synth::Compute(int frame_count) {
	stereo_out_.set_size(frame_count*2);
	float* stereo_out = stereo_out_.write_ptr();
	
	sequencer_.Compute(frame_count);
	sequencer_.StartTx();

	for (int i = 0; i < voices_; i++) {
		auto& vco = vcos_[i];
		vco.Compute(frame_count);
		vco.StartTx();
		
		auto& mono_to_stereo = mono_to_stereos_[i];
		mono_to_stereo.Compute(frame_count);
		mono_to_stereo.StartTx();

		auto& filter = filters_[i];
		filter.Compute(frame_count);
		filter.StartTx();
		
		auto& adsr = adsrs_[i];
		adsr.Compute(frame_count);
		adsr.StartTx();
	}

  mixer_.Compute(frame_count);
  mixer_.StartTx();

  // Clip.
  const float* mixer_stereo_out = mixer_.stereo_out()->read_ptr();
  for (int i = 0; i < frame_count; i++) {
    float left = mixer_stereo_out[2*i];
    stereo_out[2*i] = std::max(-1.0f, std::min(1.0f, left));
    float right = mixer_stereo_out[2*i+1];
    stereo_out[2*i+1] = std::max(-1.0f, std::min(1.0f, right));
  }
}
