#ifndef VCO_H_
#define VCO_H_

#include "base.h"
#include "node.h"

class VCO : public Node {
public:
	VCO() {}  // for creating arrays
	VCO(int sample_rate, int frames_per_chunk)
		: mono_out_(frames_per_chunk),
			seconds_per_frame_(SecondsPerFrame(sample_rate)),
			radians_(0.0f) {}
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
};

#endif  // VCO_H_
