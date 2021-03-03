#ifndef ADSR_H_
#define ADSR_H_

#include "base.h"
#include "node.h"
#include "chunk.h"

class ADSR : public Node {
public:
	ADSR() {}  // for creating arrays
	ADSR(int sample_rate, int frames_per_chunk)
		: stereo_in_(nullptr),
			stereo_out_(frames_per_chunk*2),
			duration_per_frame_(DurationPerFrame(sample_rate)),
			volume_(0.1),
			attack_(Duration(12) * Millisecond),
			decay_(Duration(200) * Millisecond),
			sustain_(0.5),
			release_(Duration(440) * Millisecond),
			state_(State::OFF),
			t_(0) {}

	void set_stereo_in(const ChunkTx<float>* stereo_in) {
		stereo_in_ = stereo_in;
	}

	void set_trigger_in(const ChunkTx<float>* trigger_in) {
		trigger_in_ = trigger_in;
	}

	const ChunkTx<float>* stereo_out() const {
		return &stereo_out_;
	}

	void StopTx() override {
		stereo_out_.Stop();
	}

	bool RxAvailable() const override {
		return stereo_in_ != nullptr && stereo_in_->available() &&
			trigger_in_ != nullptr && trigger_in_->available();
	}

	void Compute(int frame_count) override;

	void StartTx() override {
		stereo_out_.Start();
	}

private:
	enum class State {OFF, ATTACK, DECAY, SUSTAIN, RELEASE};

	Duration duration_per_frame_;
	Duration attack_;
	Duration decay_;
	float sustain_;
	Duration release_;
	float volume_;

	State state_;
	Duration t_;

	// Inputs.
	const ChunkTx<float>* stereo_in_;
	const ChunkTx<float>* trigger_in_;

	// Outputs.
	ChunkTx<float> stereo_out_;
};

#endif  // ADSR_H_
