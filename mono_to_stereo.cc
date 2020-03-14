#include "mono_to_stereo.h"

void MonoToStereo::Compute(int frame_count) {
	CHECK(mono_in_ != nullptr);
	const float* mono_in = mono_in_->read_ptr();
	CHECK(mono_in_->size() == frame_count);
	
	stereo_out_.set_size(frame_count*2);
	float* stereo_out = stereo_out_.write_ptr();

	for (int i = 0; i < frame_count; i++) {
		float sample = mono_in[i];
		stereo_out[2*i] = sample;
		stereo_out[2*i+1] = sample;
	}
}
