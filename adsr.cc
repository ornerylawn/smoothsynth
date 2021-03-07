#include "adsr.h"

#include <cmath>

bool ADSR::Rx() const override {
  return trigger_in_ != nullptr && trigger_in_->tx();
}

void ADSR::ComputeAndStartTx(int frame_count) {
  const float* trigger_in = trigger_in_->read_ptr();

  cv_out_.set_size(frame_count);
  float* cv_out = cv_out_.write_ptr();

  for (int i = 0; i < frame_count; ++i) {
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
          amp = volume_ * 1.0f;
          state_ = State::DECAY;
          t_ = Duration(0);
        } else {
          amp = volume_ * (t_ / attack_);
          t_ += duration_per_frame_;
        }
        break;
      case State::DECAY:
        if (t_ >= decay_) {
          amp = volume_ * sustain_;
          state_ = State::SUSTAIN;
        } else {
          amp = volume_ *
                (sustain_ + std::exp(-5 * (t_ / decay_)) * (1.0f - sustain_));
          t_ += duration_per_frame_;
        }
        break;
      case State::SUSTAIN:
        amp = volume_ * sustain_;
        break;
      case State::RELEASE:
        if (t_ >= release_) {
          state_ = State::OFF;
        } else {
          amp = volume_ * sustain_ * std::exp(-5 * (t_ / release_));
          t_ += duration_per_frame_;
        }
        break;
      case State::OFF:
        break;
    }

    cv_out[i] = amp;
  }
  cv_out_.set_tx(true);
}

void ADSR::StopTx() { cv_out_.set_tx(false); }