#ifndef LOWPASS_H_
#define LOWPASS_H_

#include "base.h"
#include "node.h"
#include "chunk.h"

class LowPass : public Node {
public:
  LowPass() {}  // for creating arrays
  LowPass(int sample_rate, int frames_per_chunk)
    : stereo_in_(nullptr),
      stereo_out_(2*frames_per_chunk),
      duration_per_frame_(DurationPerFrame(sample_rate)),
      last_left_sample_(0.0f),
      last_right_sample_(0.0f) {}

  void set_stereo_in(const ChunkTx<float>* stereo_in) {
    stereo_in_ = stereo_in;
  }

  const ChunkTx<float>* stereo_out() const {
    return &stereo_out_;
  }

  void StopTx() override {
    stereo_out_.Stop();
  }

  bool RxAvailable() const override {
    return stereo_in_ != nullptr && stereo_in_->available();
  }

  void Compute(int frame_count) override;

  void StartTx() override {
    stereo_out_.Start();
  }

private:
  Duration duration_per_frame_;
  float last_left_sample_;
  float last_right_sample_;

  // Inputs.
  const ChunkTx<float>* stereo_in_;

  // Outputs.
  ChunkTx<float> stereo_out_;
};

#endif  // LOWPASS_H_
