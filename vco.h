#ifndef VCO_H_
#define VCO_H_

#include "base.h"
#include "chunk.h"
#include "node.h"

class VCO : public Node {
 public:
  VCO() {}
  VCO(int sample_rate, int frames_per_chunk, float drift_offset);
  virtual ~VCO() {}

  void set_frequency_in(const ChunkTx<float>* frequency_in) {
    frequency_in_ = frequency_in;
  }

  const ChunkTx<float>* mono_out() const { return &mono_out_; }

  bool Rx() const override;
  void ComputeAndStartTx(int frame_count) override;
  void StopTx() override;

 private:
  float seconds_per_frame_;
  float radians_;
  float drift_radians_;
  float drift_f_;
  float drift_amp_;
  float drift_offset_;

  FixedArray<float> wave_table_;

  const ChunkTx<float>* frequency_in_;
  ChunkTx<float> mono_out_;
};

#endif  // VCO_H_
