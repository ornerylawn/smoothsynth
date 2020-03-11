#ifndef ADSR_H_
#define ADSR_H_

#include "base.h"
#include "node.h"

class ADSR : public Node {
public:
	ADSR() {}  // for creating arrays
	ADSR(int sample_rate, int frames_per_chunk)
		: stereo_out_(frames_per_chunk*2),
			duration_per_frame_(DurationPerFrame(sample_rate)),
			volume_(0.1),
			attack_(Duration(10) * Millisecond),
			decay_(Duration(200) * Millisecond),
			sustain_(0.4),
			release_(Duration(380) * Millisecond),
			state_(State::OFF), t_(0) {}

	void set_maybe_stereo_in(const Optional<ArrayView<float>>* maybe_stereo_in) {
		maybe_stereo_in_ = maybe_stereo_in;
	}

	void set_maybe_trigger_in(const Optional<ArrayView<float>>* maybe_trigger_in) {
		maybe_trigger_in_ = maybe_trigger_in;
	}

	const Optional<ArrayView<float>>* maybe_stereo_out() const {
		return &maybe_stereo_out_;
	}

	void MakeOutputsNil() override {
		maybe_stereo_out_ = Nil<ArrayView<float>>();
	}

	bool inputs_available() const override {
		return !maybe_stereo_in_->is_nil();
	}

	void Compute(int frame_count) override;

private:
	enum class State {OFF, ATTACK, DECAY, SUSTAIN, RELEASE};
	const Optional<ArrayView<float>>* maybe_stereo_in_;
	const Optional<ArrayView<float>>* maybe_trigger_in_;
	FixedArray<float> stereo_out_;
	Optional<ArrayView<float>> maybe_stereo_out_;

	Duration duration_per_frame_;
	Duration attack_;
	Duration decay_;
	float sustain_;
	Duration release_;
	float volume_;

	State state_;
	Duration t_;
};

#endif  // ADSR_H_
