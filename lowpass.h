#ifndef LOWPASS_H_
#define LOWPASS_H_

#include "base.h"
#include "node.h"

class LowPass : public Node {
public:
	LowPass() {}  // for creating arrays
	LowPass(int sample_rate, int frames_per_chunk)
		: mono_out_(frames_per_chunk),
			duration_per_frame_(DurationPerFrame(sample_rate)),
			last_sample_(0.0f) {}

	void set_maybe_mono_in(const Optional<ArrayView<float>>* maybe_mono_in) {
		maybe_mono_in_ = maybe_mono_in;
	}

	const Optional<ArrayView<float>>* maybe_mono_out() const {
		return &maybe_mono_out_;
	}

	void MakeOutputsNil() override {
		maybe_mono_out_ = Nil<ArrayView<float>>();
	}

	bool inputs_available() const override {
		return !maybe_mono_in_->is_nil();
	}

	void Compute(int frame_count) override;

private:
	const Optional<ArrayView<float>>* maybe_mono_in_;
	FixedArray<float> mono_out_;
	Optional<ArrayView<float>> maybe_mono_out_;

	Duration duration_per_frame_;
	float last_sample_;
};

#endif  // LOWPASS_H_
