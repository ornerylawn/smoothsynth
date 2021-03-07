#ifndef ADSR_H_
#define ADSR_H_

#include "base.h"
#include "chunk.h"
#include "node.h"

class ADSR : public Node {
 public:
  ADSR() {}  // for creating arrays
  ADSR(int sample_rate, int frames_per_chunk)
      : trigger_in_(nullptr),
        cv_out_(frames_per_chunk),
        duration_per_frame_(DurationPerFrame(sample_rate)),
        volume_(0.3),
        attack_(Duration(12) * Millisecond),
        decay_(Duration(200) * Millisecond),
        sustain_(0.4),
        release_(Duration(540) * Millisecond),
        state_(State::OFF),
        t_(0) {}

  void set_trigger_in(const ChunkTx<float>* trigger_in) {
    trigger_in_ = trigger_in;
  }

  const ChunkTx<float>* cv_out() const { return &cv_out_; }

  bool Rx() const override;
  void ComputeAndStartTx(int frame_count) override;
  void StopTx() override;

 private:
  const ChunkTx<float>* trigger_in_;
  ChunkTx<float> cv_out_;

  Duration duration_per_frame_;
  Duration attack_;
  Duration decay_;
  float sustain_;
  Duration release_;
  float volume_;

  enum class State { OFF, ATTACK, DECAY, SUSTAIN, RELEASE };
  State state_;
  Duration t_;
};

#endif  // ADSR_H_
