#ifndef VCO_H_
#define VCO_H_

#include "base.h"
#include "node.h"

class VCO : public Node {
public:
	VCO() {}  // for creating arrays
	VCO(int sample_rate, int frames_per_chunk, float drift_offset)
		: mono_out_(frames_per_chunk),
			seconds_per_frame_(SecondsPerFrame(sample_rate)),
			radians_(0.0f),
			drift_radians_(0.0f),
			drift_f_(0.7f),
			drift_amp_(0.5f),
			drift_offset_(drift_offset) {}
	virtual ~VCO() {}

	void set_maybe_frequency_in(const Optional<ArrayView<float>>* maybe_frequency_in) {
		maybe_frequency_in_ = maybe_frequency_in;
	}

	const Optional<ArrayView<float>>* maybe_mono_out() const {
		return &maybe_mono_out_;
	}

	void MakeOutputsNil() override {
		maybe_mono_out_ = Nil<ArrayView<float>>();
	}

	bool inputs_available() const override {
		return !maybe_frequency_in_->is_nil();
	}

	void Compute(int frame_count) override;

private:
	const Optional<ArrayView<float>>* maybe_frequency_in_;
	FixedArray<float> mono_out_;
	Optional<ArrayView<float>> maybe_mono_out_;

	float seconds_per_frame_;
	float radians_;

	float drift_radians_;
	float drift_f_;
	float drift_amp_;
	float drift_offset_;
};

#endif  // VCO_H_
