#include "vco.h"

#include <cmath>

void VCO::Compute(int frame_count) {
	// TODO: all components should be able to function without all of
	// their inputs connected (frequency should be const or something).
	CHECK(frequency_in_ != nullptr);
	const float* frequency_in = frequency_in_->read_ptr();
	CHECK(frequency_in_->size() == frame_count);

	mono_out_.set_size(frame_count);
	float* mono_out = mono_out_.write_ptr();

	// Sine.
	// for (int i = 0; i < frame_count; i++) {
	// 	mono_out_[i] = std::sin(radians_);
	// 	float dr = RadiansPerFrame(frequency_in[i], seconds_per_frame_);
	// 	radians_ = WrapRadians(radians_ + dr);
	// }

	// Saw.
	// TODO: bandlimited synthesis that sounds like an oberheim.
	for (int i = 0; i < frame_count; i++) {
		float drift = std::sin(drift_radians_);
		float f = frequency_in[i] + drift_amp_*drift + drift_offset_;
		float sum = 0.0;
		for (int j = 1; j <= 20; j++) {
			sum += std::pow(-1, j) * std::sin(radians_ * j) / j;
		}
		mono_out[i] = 2 * sum / PI;
		float dr = RadiansPerFrame(f, seconds_per_frame_);
		radians_ = WrapRadians(radians_ + dr);
		float drift_dr = RadiansPerFrame(drift_f_, seconds_per_frame_);
		drift_radians_ = WrapRadians(drift_radians_ + drift_dr);
	}
}
