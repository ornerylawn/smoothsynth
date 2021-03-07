#ifndef VCF_H_
#define VCF_H_

#include "base.h"
#include "chunk.h"
#include "node.h"

class VCF : public Node {
 public:
  VCF() {}
  VCF(int sample_rate, int frames_per_chunk)
      : stereo_in_(nullptr),
        stereo_out_(2 * frames_per_chunk),
        duration_per_frame_(DurationPerFrame(sample_rate)),
        last_left_sample_(0.0f),
        last_right_sample_(0.0f) {}

  void set_stereo_in(const ChunkTx<float>* stereo_in) {
    stereo_in_ = stereo_in;
  }

  const ChunkTx<float>* stereo_out() const { return &stereo_out_; }

  bool Rx() const override;
  void ComputeAndStartTx(int frame_count) override;
  void StopTx() override { stereo_out_.set_tx(false); }

 private:
  Duration duration_per_frame_;
  float last_left_sample_, last_right_sample_;
  const ChunkTx<float>* stereo_in_;
  ChunkTx<float> stereo_out_;
};

#endif  // VCF_H_
