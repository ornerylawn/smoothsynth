#ifndef PORT_SYSTEM_H_
#define PORT_SYSTEM_H_

#include <functional>
#include <portaudio.h>
#include <portmidi.h>

#include "chunk.h"

class PortSystem {
public:
	PortSystem(int midi_in_buffer_size,
						 const std::string& midi_in_name,
						 int sample_rate,
						 int frames_per_chunk,
						 std::function<void(int)> callback)
		: midi_in_(midi_in_buffer_size),
			stereo_out_(nullptr),
			midi_in_name_(midi_in_name),
			sample_rate_(sample_rate),
			frames_per_chunk_(frames_per_chunk),
			callback_(callback) {}

	~PortSystem();

	const ChunkTx<PmEvent>* midi_in() { return &midi_in_; }

	void set_stereo_out(const ChunkTx<float>* stereo_out) {
		stereo_out_ = stereo_out;
	}

	Optional<Error> Start();

	void AudioDriverCallback(const float* in,
													 float* out,
													 int frame_count);

private:
	std::string midi_in_name_;
	int sample_rate_;
	int frames_per_chunk_;

	PortMidiStream* midi_in_stream_;
	PaStream* audio_out_stream_;

	bool midi_initialized_;
	bool midi_in_stream_opened_;
	bool audio_initialized_;
	bool audio_out_stream_opened_;
	bool audio_out_stream_started_;

	ChunkTx<PmEvent> midi_in_;
	const ChunkTx<float>* stereo_out_;

	std::function<void(int)> callback_;
};

#endif  // PORT_SYSTEM_H_
