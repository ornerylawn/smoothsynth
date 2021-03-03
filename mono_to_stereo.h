#ifndef MONO_TO_STEREO_H_
#define MONO_TO_STEREO_H_

#include "base.h"
#include "node.h"
#include "chunk.h"

class MonoToStereo : public Node {
public:
  MonoToStereo() {}  // for creating arrays
  MonoToStereo(int sample_rate, int frames_per_chunk)
    : mono_in_(nullptr),
      stereo_out_(frames_per_chunk*2) {}
  virtual ~MonoToStereo() {}

  void set_mono_in(const ChunkTx<float>* mono_in) {
    mono_in_ = mono_in;
  }

  const ChunkTx<float>* stereo_out() const {
    return &stereo_out_;
  }

  void StopTx() override {
    stereo_out_.Stop();
  }

  bool RxAvailable() const override {
    return mono_in_ != nullptr && mono_in_->available();
  }

  void Compute(int frame_count) override;

  void StartTx() override {
    stereo_out_.Start();
  }

private:
  const ChunkTx<float>* mono_in_;
  ChunkTx<float> stereo_out_;
};

#endif  // MONO_TO_STEREO_H_
