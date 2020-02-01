#include "smooth_synth.h"

#include <cmath>

SmoothSynth::SmoothSynth(int sample_rate, int frames_per_chunk, int voices)
	: voices_(voices),
		sequencer_(sample_rate, frames_per_chunk, voices),
		vcos_(voices, VCO(sample_rate, frames_per_chunk)),
		mono_to_stereos_(voices, MonoToStereo(sample_rate, frames_per_chunk)),
		adsrs_(voices, ADSR(sample_rate, frames_per_chunk)),
		stereo_out_(frames_per_chunk*2) {
	for (int i = 0; i < voices_; i++) {
		vcos_[i].set_maybe_frequency_in(sequencer_.maybe_frequency_outs(i));
		mono_to_stereos_[i].set_maybe_mono_in(vcos_[i].maybe_mono_out());
		adsrs_[i].set_maybe_stereo_in(mono_to_stereos_[i].maybe_stereo_out());
		adsrs_[i].set_maybe_trigger_in(sequencer_.maybe_trigger_outs(i));
	}
}

void SmoothSynth::MakeOutputsNil() {
	sequencer_.MakeOutputsNil();
	for (int i = 0; i < voices_; i++) {
		vcos_[i].MakeOutputsNil();
		mono_to_stereos_[i].MakeOutputsNil();
		adsrs_[i].MakeOutputsNil();
	}
	maybe_stereo_out_ = Nil<ArrayView<float>>();
}

void SmoothSynth::Compute(int frame_count) {
	sequencer_.Compute(frame_count);

	for (int i = 0; i < frame_count; i++) {
		stereo_out_[2*i] = 0.0f;
		stereo_out_[2*i+1] = 0.0f;
	}

	for (int i = 0; i < voices_; i++) {
		vcos_[i].Compute(frame_count);
		mono_to_stereos_[i].Compute(frame_count);
		adsrs_[i].Compute(frame_count);

		ArrayView<float> adsr_stereo_out = adsrs_[i].maybe_stereo_out()->ValueOrDie();
		CHECK(adsr_stereo_out.size() == frame_count*2);
		for (int i = 0; i < frame_count; i++) {
			stereo_out_[2*i] += adsr_stereo_out[2*i];
			stereo_out_[2*i+1] += adsr_stereo_out[2*i+1];
		}
	}

	for (int i = 0; i < frame_count; i++) {
		float& left = stereo_out_[2*i];
		left = std::max(-1.0f, std::min(left, 1.0f));
		float& right = stereo_out_[2*i+1];
		right = std::max(-1.0f, std::min(right, 1.0f));
	}
	maybe_stereo_out_ = AsOptional(ArrayView<float>(&stereo_out_, 0, frame_count*2));
}
