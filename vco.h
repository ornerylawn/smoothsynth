#ifndef VCO_H_
#define VCO_H_

#include "base.h"
#include "node.h"
#include "chunk.h"

class VCO : public Node {
public:
	VCO() {}  // for creating arrays
	VCO(int sample_rate, int frames_per_chunk, float drift_offset)
		: frequency_in_(nullptr),
			mono_out_(frames_per_chunk),
			seconds_per_frame_(SecondsPerFrame(sample_rate)),
			radians_(0.0f),
			drift_radians_(0.0f),
			drift_f_(0.7f),
			drift_amp_(0.5f),
			drift_offset_(drift_offset) {}
	virtual ~VCO() {}

	void set_frequency_in(const ChunkTx<float>* frequency_in) {
		frequency_in_ = frequency_in;
	}

	const ChunkTx<float>* mono_out() const {
		return &mono_out_;
	}

	void StopTx() override {
		mono_out_.Stop();
	}

	bool RxAvailable() const override {
		return frequency_in_ != nullptr && frequency_in_->available();
	}

	void Compute(int frame_count) override;

	void StartTx() override {
		mono_out_.Start();
	}

private:
	float seconds_per_frame_;
	float radians_;
	float drift_radians_;
	float drift_f_;
	float drift_amp_;
	float drift_offset_;

	// Input.
	const ChunkTx<float>* frequency_in_;

	// Output.
  ChunkTx<float> mono_out_;
};

#endif  // VCO_H_
