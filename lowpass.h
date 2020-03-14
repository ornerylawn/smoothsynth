#ifndef LOWPASS_H_
#define LOWPASS_H_

#include "base.h"
#include "node.h"
#include "chunk.h"

class LowPass : public Node {
public:
	LowPass() {}  // for creating arrays
	LowPass(int sample_rate, int frames_per_chunk)
		: mono_in_(nullptr),
			mono_out_(frames_per_chunk),
			duration_per_frame_(DurationPerFrame(sample_rate)),
			last_sample_(0.0f) {}

	void set_mono_in(const ChunkTx<float>* mono_in) {
		mono_in_ = mono_in;
	}

	const ChunkTx<float>* mono_out() const {
		return &mono_out_;
	}

	void StopTx() override {
		mono_out_.Stop();
	}

	bool RxAvailable() const override {
		return mono_in_ != nullptr && mono_in_->available();
	}

	void Compute(int frame_count) override;

	void StartTx() override {
		mono_out_.Start();
	}

private:
	Duration duration_per_frame_;
	float last_sample_;

	// Inputs.
	const ChunkTx<float>* mono_in_;

	// Outputs.
	ChunkTx<float> mono_out_;
};

#endif  // LOWPASS_H_
