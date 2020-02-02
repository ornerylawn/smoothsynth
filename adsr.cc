#include "adsr.h"

#include <cmath>

void ADSR::Compute(int frame_count) {
	ArrayView<float> stereo_in = maybe_stereo_in_->ValueOrDie();
	ArrayView<float> trigger_in = maybe_trigger_in_->ValueOrDie();
	CHECK(stereo_in.size() == frame_count*2);
	CHECK(trigger_in.size() == frame_count);

	float volume = 0.4;

	for (int i = 0; i < frame_count; i++) {
		float trigger = trigger_in[i];
		if (1.0 <= trigger) {  // ON
			state_ = State::ATTACK;
			t_ = Duration(0);
		} else if (trigger <= -1.0) {  // OFF
			state_ = State::RELEASE;
			t_ = Duration(0);
		}

		float amp = 0.0;
		switch (state_) {
		case State::ATTACK:
			if (t_ >= attack_) {
				amp = volume * 1.0f;
				state_ = State::DECAY;
				t_ = Duration(0);
			} else {
				amp = volume * (t_ / attack_);
				t_ += duration_per_frame_;
			}
			break;
		case State::DECAY:
			if (t_ >= decay_) {
				amp = volume * sustain_;
				state_ = State::SUSTAIN;
			} else {
				amp = volume * (sustain_ + std::exp(-5*(t_ / decay_)) * (1.0f - sustain_));
				t_ += duration_per_frame_;
			}
			break;
		case State::SUSTAIN:
			amp = volume * sustain_;
			break;
		case State::RELEASE:
			if (t_ >= release_) {
				state_ = State::OFF;
			} else {
				amp = volume * sustain_ * std::exp(-5*(t_ / release_));
				t_ += duration_per_frame_;
			}
			break;
		case State::OFF:
			break;
		}

		stereo_out_[2*i] = amp * stereo_in[2*i];
		stereo_out_[2*i+1] = amp * stereo_in[2*i+1];
	}

	maybe_stereo_out_ = AsOptional(ArrayView<float>(&stereo_out_, 0, frame_count*2));
}
