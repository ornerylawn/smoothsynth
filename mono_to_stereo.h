#ifndef MONO_TO_STEREO_H_
#define MONO_TO_STEREO_H_

#include "base.h"
#include "node.h"

class MonoToStereo : public Node {
public:
	MonoToStereo() {}  // for creating arrays
	MonoToStereo(int sample_rate, int frames_per_chunk)
		: stereo_out_(frames_per_chunk*2) {}
	virtual ~MonoToStereo() {}

	void set_maybe_mono_in(const Optional<ArrayView<float>>* maybe_mono_in) {
		maybe_mono_in_ = maybe_mono_in;
	}

	const Optional<ArrayView<float>>* maybe_stereo_out() const {
		return &maybe_stereo_out_;
	}

	void MakeOutputsNil() override {
		maybe_stereo_out_ = Nil<ArrayView<float>>();
	}

	bool inputs_available() const override {
		return !maybe_mono_in_->is_nil();
	}

	void Compute(int frame_count) override;

private:
	const Optional<ArrayView<float>>* maybe_mono_in_;
	FixedArray<float> stereo_out_;
	Optional<ArrayView<float>> maybe_stereo_out_;
};

#endif  // MONO_TO_STEREO_H_
