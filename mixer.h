#ifndef MIXER_H_
#define MIXER_H_

#include "base.h"
#include "chunk.h"
#include "node.h"

class Mixer : Node {
public:
  Mixer() {}
  Mixer(int sample_rate, int frames_per_chunk, int tracks)
    : stereo_ins_(tracks),
      stereo_out_(2*frames_per_chunk) {
    for (int i = 0; i < tracks; i++) {
      stereo_ins_[i] = nullptr;
    }
  }

  void set_stereo_in(int i, const ChunkTx<float>* stereo_in) {
    stereo_ins_[i] = stereo_in;
  }

  const ChunkTx<float>* stereo_out() const { return &stereo_out_; }

  void StopTx() override { stereo_out_.Stop(); }

  bool RxAvailable() const override {
    for (int i = 0; i < stereo_ins_.size(); i++) {
      if (stereo_ins_[i] == nullptr || !stereo_ins_[i]->available()) {
        return false;
      }
    }
    return true;
  }

  void Compute(int frame_count) override;

  void StartTx() override {
    stereo_out_.Start();
  }

private:
  FixedArray<const ChunkTx<float>*> stereo_ins_;
  ChunkTx<float> stereo_out_;
};

#endif  // MIXER_H_
