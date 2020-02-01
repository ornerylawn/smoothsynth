#include "mono_to_stereo.h"

void MonoToStereo::Compute(int frame_count) {
	ArrayView<float> mono_in = maybe_mono_in_->ValueOrDie();
	CHECK(mono_in.size() == frame_count);

	for (int i = 0; i < frame_count; i++) {
		float sample = mono_in[i];
		stereo_out_[2*i] = sample;
		stereo_out_[2*i+1] = sample;
	}

	maybe_stereo_out_ = AsOptional(ArrayView<float>(&stereo_out_, 0, frame_count*2));
}
