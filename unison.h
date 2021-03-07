#ifndef UNISON_H_
#define UNISON_H_

#include "base.h"
#include "chunk.h"
#include "node.h"

class Unison : public Node {
 public:
  Unison() {}
  Unison(int sample_rate, int frames_per_chunk)
      : mono_in_(nullptr), stereo_out_(frames_per_chunk * 2) {}
  virtual ~Unison() {}

  void set_mono_in(const ChunkTx<float>* mono_in) { mono_in_ = mono_in; }
  const ChunkTx<float>* stereo_out() const { return &stereo_out_; }

  bool Rx() const override;
  void ComputeAndStartTx(int frame_count) override;
  void StopTx() override;

 private:
  const ChunkTx<float>* mono_in_;
  ChunkTx<float> stereo_out_;
};

#endif  // UNISON_H_
