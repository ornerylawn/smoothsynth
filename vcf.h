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
        sample_rate_(sample_rate),
        duration_per_frame_(DurationPerFrame(sample_rate)),
        cutoff_(4000.0f),
        left_notch_(0.0f),
        left_low_(0.0f),
        left_high_(0.0f),
        left_band_(0.0f) {}

  void set_stereo_in(const ChunkTx<float>* stereo_in) {
    stereo_in_ = stereo_in;
  }

  void set_cutoff(float cutoff) { cutoff_ = cutoff; }

  const ChunkTx<float>* stereo_out() const { return &stereo_out_; }

  bool Rx() const override;
  void ComputeAndStartTx(int frame_count) override;
  void StopTx() override;

 private:
  const ChunkTx<float>* stereo_in_;
  ChunkTx<float> stereo_out_;
  float sample_rate_;
  Duration duration_per_frame_;
  float cutoff_;

  float left_notch_;
  float left_low_;
  float left_high_;
  float left_band_;
};

#endif  // VCF_H_
