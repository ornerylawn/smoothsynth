#ifndef SYNTH_H_
#define SYNTH_H_

#include "adsr.h"
#include "lowpass.h"
#include "mixer.h"
#include "mono_to_stereo.h"
#include "node.h"
#include "sequencer.h"
#include "vco.h"

class Synth : Node {
public:
	Synth(int sample_rate, int frames_per_chunk, int voices);
	virtual ~Synth() {}

	void set_midi_in(const ChunkTx<PmEvent>* midi_in) {
		sequencer_.set_midi_in(midi_in);
	}
	
	const ChunkTx<float>* stereo_out() const {
		return &stereo_out_;
	}

	void StopTx() override {
		stereo_out_.Stop();
    mixer_.StopTx();
		for (int i = 0; i < voices_; i++) {
			adsrs_[i].StopTx();
			mono_to_stereos_[i].StopTx();
			filters_[i].StopTx();
			vcos_[i].StopTx();
		}
		sequencer_.StopTx();
	}
	
	bool RxAvailable() const override {
		return sequencer_.RxAvailable();
	}
	
	void Compute(int frame_count) override;

	void StartTx() override {
		stereo_out_.Start();
	}

private:
	int voices_;

	// Components.
	Sequencer sequencer_;
	FixedArray<VCO> vcos_;
	FixedArray<LowPass> filters_;
	FixedArray<MonoToStereo> mono_to_stereos_;
	FixedArray<ADSR> adsrs_;
  Mixer mixer_;

	// Outputs.
	ChunkTx<float> stereo_out_;
};

#endif  // SYNTH_H_
