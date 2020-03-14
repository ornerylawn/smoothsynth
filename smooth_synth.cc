#include "smooth_synth.h"

#include <cmath>

SmoothSynth::SmoothSynth(int sample_rate, int frames_per_chunk, int voices)
	: voices_(voices),
		sequencer_(sample_rate, frames_per_chunk, voices),
		vcos_(voices, VCO(sample_rate, frames_per_chunk, 0.0)),
		filters_(voices, LowPass(sample_rate, frames_per_chunk)),
		mono_to_stereos_(voices, MonoToStereo(sample_rate, frames_per_chunk)),
		adsrs_(voices, ADSR(sample_rate, frames_per_chunk)),
		stereo_out_(frames_per_chunk*2) {
	for (int i = 0; i < voices_; i++) {
		vcos_[i].set_frequency_in(sequencer_.frequency_outs(i));
		filters_[i].set_mono_in(vcos_[i].mono_out());
		mono_to_stereos_[i].set_mono_in(filters_[i].mono_out());
		adsrs_[i].set_stereo_in(mono_to_stereos_[i].stereo_out());
		adsrs_[i].set_trigger_in(sequencer_.trigger_outs(i));
	}
}

void SmoothSynth::Compute(int frame_count) {
	stereo_out_.set_size(frame_count*2);
	float* stereo_out = stereo_out_.write_ptr();
	
	sequencer_.Compute(frame_count);
	sequencer_.StartTx();

	for (int i = 0; i < frame_count; i++) {
		stereo_out[2*i] = 0.0f;
	 	stereo_out[2*i+1] = 0.0f;
	}

	for (int i = 0; i < voices_; i++) {
		auto& vco = vcos_[i];
		vco.Compute(frame_count);
		vco.StartTx();
		
		auto& filter = filters_[i];
		filter.Compute(frame_count);
		filter.StartTx();
		
		auto& mono_to_stereo = mono_to_stereos_[i];
		mono_to_stereo.Compute(frame_count);
		mono_to_stereo.StartTx();
		
		auto& adsr = adsrs_[i];
		adsr.Compute(frame_count);
		adsr.StartTx();

		const float* adsr_stereo_out = adsr.stereo_out()->read_ptr();
		CHECK(adsr.stereo_out()->size() == frame_count*2);
		for (int j = 0; j < frame_count; j++) {
			stereo_out[2*j] += adsr_stereo_out[2*j];
			stereo_out[2*j+1] += adsr_stereo_out[2*j+1];
		}
	}

	// Clip.
	for (int i = 0; i < frame_count; i++) {
		float& left = stereo_out[2*i];
		left = std::max(-1.0f, std::min(left, 1.0f));
		float& right = stereo_out[2*i+1];
		right = std::max(-1.0f, std::min(right, 1.0f));
	}
}
