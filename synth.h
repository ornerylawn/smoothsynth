#ifndef SYNTH_H_
#define SYNTH_H_

#include "adsr.h"
#include "mixer.h"
#include "node.h"
#include "sequencer.h"
#include "unison.h"
#include "vca.h"
#include "vcf.h"
#include "vco.h"

class Synth : Node {
 public:
  Synth(int sample_rate, int frames_per_chunk, int voices);
  virtual ~Synth() {}

  void set_midi_in(const ChunkTx<PmEvent>* midi_in) {
    sequencer_.set_midi_in(midi_in);
  }

  const ChunkTx<float>* stereo_out() const { return &stereo_out_; }

  bool Rx() const override;
  void ComputeAndStartTx(int frame_count) override;
  void StopTx() override;

 private:
  int voices_;

  // Components.
  Sequencer sequencer_;
  FixedArray<ADSR> adsrs_;
  FixedArray<VCO> vcos_;
  FixedArray<Unison> unisons_;
  FixedArray<VCF> filters_;
  FixedArray<VCA> vcas_;
  Mixer mixer_;

  // Outputs.
  ChunkTx<float> stereo_out_;
};

#endif  // SYNTH_H_
