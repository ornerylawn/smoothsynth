#include "lowpass.h"

void LowPass::Compute(int frame_count) {
	ArrayView<float> mono_in = maybe_mono_in_->ValueOrDie();
	CHECK(mono_in.size() == frame_count);

	mono_out_[0] = mono_in[0] + last_sample_;
	for (int i = 1; i < frame_count; i++) {
		mono_out_[i] = mono_in[i] + mono_in[i-1];
	}
	last_sample_ = mono_in[frame_count-1];

	maybe_mono_out_ = AsOptional(ArrayView<float>(&mono_out_, 0, frame_count));
}
