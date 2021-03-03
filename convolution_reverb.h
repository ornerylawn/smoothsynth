#ifndef CONVOLUTION_REVERB_H_
#define CONVOLUTION_REVERB_H_

#include "chunk.h"
#include "node.h"

class ConvolutionReverb : public Node {
public:
  ConvolutionReverb() {}
  ConvolutionReverb(int sample_rate, int frames_per_chunk);
  virtual ~ConvolutionReverb() {}

  void set_stereo_in(const ChunkTx<float>* stereo_in) {
    stereo_in_ = stereo_in;
  }

  const ChunkTx<float>* stereo_out() const {
    return &stereo_out_;
  }

  void StopTx() override {
    stereo_out_.Stop();
  }

  bool RxAvailable() const override {
    return stereo_in_ != nullptr && stereo_in_->available();
  }

  void Compute(int frame_count) override;

  void StartTx() override {
    stereo_out_.Start();
  }
  
private:
  Duration duration_per_frame_;
  Duration decay_;
  const ChunkTx<float>* stereo_in_;
  ChunkTx<float> stereo_out_;
  FixedArray<float> impulse_response_;
  FixedArray<float> input_ring_;
  int w_;
  int wrap_mask_;
};

#endif  // CONVOLUTION_REVERB_H_
