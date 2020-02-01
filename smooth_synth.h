#ifndef SMOOTH_SYNTH_H_
#define SMOOTH_SYNTH_H_

#include "synth.h"
#include "sequencer.h"
#include "vco.h"
#include "mono_to_stereo.h"
#include "adsr.h"

class SmoothSynth : public Synth {
public:
	SmoothSynth(int sample_rate, int frames_per_chunk, int voices);
	virtual ~SmoothSynth() {}

	void set_maybe_midi_in(const Optional<ArrayView<PmEvent>>* maybe_midi_in) override {
		sequencer_.set_maybe_midi_in(maybe_midi_in);
	}
	
	const Optional<ArrayView<float>>* maybe_stereo_out() const override {
		return &maybe_stereo_out_;
	}

	void MakeOutputsNil() override;
	
	bool inputs_available() const override {
		return sequencer_.inputs_available();
	}
	
	void Compute(int frame_count) override;

private:
	int voices_;
	Sequencer sequencer_;
	FixedArray<VCO> vcos_;
	FixedArray<MonoToStereo> mono_to_stereos_;
	FixedArray<ADSR> adsrs_;

	FixedArray<float> stereo_out_;
	Optional<ArrayView<float>> maybe_stereo_out_;
};

#endif  // SMOOTH_SYNTH_H_
