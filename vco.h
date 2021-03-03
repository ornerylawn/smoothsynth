#ifndef VCO_H_
#define VCO_H_

#include "base.h"
#include "chunk.h"
#include "node.h"

class VCO : public Node {
public:
  VCO() {}  // for creating arrays
  VCO(int sample_rate, int frames_per_chunk, float drift_offset);
  virtual ~VCO() {}

  void set_frequency_in(const ChunkTx<float>* frequency_in) {
    frequency_in_ = frequency_in;
  }

  const ChunkTx<float>* mono_out() const {
    return &mono_out_;
  }

  void StopTx() override {
    mono_out_.Stop();
  }

  bool RxAvailable() const override {
    return frequency_in_ != nullptr && frequency_in_->available();
  }

  void Compute(int frame_count) override;

  void StartTx() override {
    mono_out_.Start();
  }

private:
  float seconds_per_frame_;
  float radians_;
  float drift_radians_;
  float drift_f_;
  float drift_amp_;
  float drift_offset_;

  FixedArray<float> wave_table_;

  // Input.
  const ChunkTx<float>* frequency_in_;

  // Output.
  ChunkTx<float> mono_out_;
};

#endif  // VCO_H_
