#ifndef MIXER_H_
#define MIXER_H_

#include "base.h"
#include "chunk.h"
#include "node.h"

class Mixer : Node {
 public:
  Mixer() {}
  Mixer(int sample_rate, int frames_per_chunk, int tracks)
      : stereo_ins_(tracks), stereo_out_(2 * frames_per_chunk) {
    for (int i = 0; i < tracks; i++) {
      stereo_ins_[i] = nullptr;
    }
  }

  void set_stereo_in(int i, const ChunkTx<float>* stereo_in) {
    stereo_ins_[i] = stereo_in;
  }

  const ChunkTx<float>* stereo_out() const { return &stereo_out_; }

  bool Rx() const override;
  void ComputeAndStartTx(int frame_count) override;
  void StopTx() override;

 private:
  FixedArray<const ChunkTx<float>*> stereo_ins_;
  ChunkTx<float> stereo_out_;
};

#endif  // MIXER_H_
