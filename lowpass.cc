#include "lowpass.h"

void LowPass::Compute(int frame_count) {
	CHECK(mono_in_ != nullptr);
	const float* mono_in = mono_in_->read_ptr();
	CHECK(mono_in_->size() == frame_count);

	mono_out_.set_size(frame_count);
	float* mono_out = mono_out_.write_ptr();

	mono_out[0] = mono_in[0] + last_sample_;
	for (int i = 1; i < frame_count; i++) {
		mono_out[i] = mono_in[i] + mono_in[i-1];
	}
	last_sample_ = mono_in[frame_count-1];
}
