#ifndef VCA_H_
#define VCA_H_

#include "node.h"

class VCA : Node {
 public:
  VCA() {}
  VCA(int sample_rate, int frames_per_chunk)
      : stereo_out_(frames_per_chunk * 2) {}
  ~VCA() {}

  void set_stereo_in(const ChunkTx<float>* stereo_in) {
    stereo_in_ = stereo_in;
  }
  void set_cv_in(const ChunkTx<float>* cv_in) { cv_in_ = cv_in; }
  const ChunkTx<float>* stereo_out() { return stereo_out_; }

  bool Rx() const override;
  void ComputeAndStartTx(int frame_count) override;
  void StopTx() override;

 private:
  const ChunkTx<float>* stereo_in_;
  const ChunkTx<float>* cv_in_;
  ChunkTx<float> stereo_out_;
};

#endif  // VCA_H_