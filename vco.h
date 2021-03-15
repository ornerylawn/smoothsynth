#ifndef VCO_H_
#define VCO_H_

#include "base.h"
#include "chunk.h"
#include "node.h"

class VCO : public Node {
 public:
  VCO() {}
  VCO(int sample_rate, int frames_per_chunk, float drift_cents);
  virtual ~VCO() {}

  void set_cv_in(const ChunkTx<float>* cv_in) { cv_in_ = cv_in; }
  const ChunkTx<float>* mono_out() const { return &mono_out_; }

  bool Rx() const override;
  void ComputeAndStartTx(int frame_count) override;
  void StopTx() override;

 private:
  const float drift_cv_;
  const float seconds_per_frame_;

  const ChunkTx<float>* cv_in_;
  ChunkTx<float> mono_out_;

  FixedArray<float> wave_table_;
  float radial_fraction_;  // [0.0, 1.0)
};

#endif  // VCO_H_
