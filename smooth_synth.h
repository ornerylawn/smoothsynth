#ifndef SMOOTH_SYNTH_H_
#define SMOOTH_SYNTH_H_

#include "synth.h"
#include "sequencer.h"
#include "vco.h"
#include "mono_to_stereo.h"
#include "adsr.h"
#include "lowpass.h"

class SmoothSynth : public Synth {
public:
	SmoothSynth(int sample_rate, int frames_per_chunk, int voices);
	virtual ~SmoothSynth() {}

	void set_midi_in(const ChunkTx<PmEvent>* midi_in) override {
		sequencer_.set_midi_in(midi_in);
	}
	
	const ChunkTx<float>* stereo_out() const override {
		return &stereo_out_;
	}

	void StopTx() override {
		stereo_out_.Stop();
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

	// Outputs.
	ChunkTx<float> stereo_out_;
};

#endif  // SMOOTH_SYNTH_H_
