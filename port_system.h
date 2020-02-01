#ifndef PORT_SYSTEM_H_
#define PORT_SYSTEM_H_

#include <portaudio.h>
#include <portmidi.h>

#include "synth.h"

class PortSystem {
public:
	PortSystem(int midi_in_buffer_size,
						 const std::string& midi_in_name,
						 int sample_rate,
						 int frames_per_chunk,
						 Synth* synth)
		: midi_in_buffer_(midi_in_buffer_size),
			midi_in_name_(midi_in_name),
			sample_rate_(sample_rate),
			frames_per_chunk_(frames_per_chunk),
			synth_(synth) {}

	~PortSystem();

	Optional<Error> Start();

	void AudioDriverCallback(const float* in,
													 float* out,
													 int frame_count);

private:
	std::string midi_in_name_;
	int sample_rate_;
	int frames_per_chunk_;

	FixedArray<PmEvent> midi_in_buffer_;
	PortMidiStream* midi_in_stream_;
	PaStream* audio_out_stream_;

	Synth* synth_;
	Optional<ArrayView<PmEvent>> maybe_midi_in_;
	const Optional<ArrayView<float>>* maybe_stereo_out_;

	bool midi_initialized_;
	bool midi_in_stream_opened_;
	bool audio_initialized_;
	bool audio_out_stream_opened_;
	bool audio_out_stream_started_;
};

#endif  // PORT_SYSTEM_H_
