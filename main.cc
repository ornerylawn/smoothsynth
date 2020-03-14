#include "base.h"
#include "port_system.h"
#include "smooth_synth.h"

Synth* g_synth = nullptr;

void AudioDriverCallback(int frame_count) {
	CHECK(g_synth != nullptr);
	g_synth->StopTx();
	CHECK(g_synth->RxAvailable());
	g_synth->Compute(frame_count);
	g_synth->StartTx();
	CHECK(g_synth->stereo_out()->available());
}

Optional<Error> run(const std::string& midi_in_name) {
	int midi_in_buffer_size = 32;
	int sample_rate = 44100;
	int frames_per_chunk = 1024;
	int voices = 8;

	PortSystem sys(midi_in_buffer_size,
								 midi_in_name,
								 sample_rate,
								 frames_per_chunk,
								 AudioDriverCallback);

	SmoothSynth synth(sample_rate,
										frames_per_chunk,
										voices);
	synth.set_midi_in(sys.midi_in());
	sys.set_stereo_out(synth.stereo_out());
	g_synth = &synth;

	MUST(sys.Start());

	// TODO: Block here and wait for stop control. There should be a
	// lameduck period where audio is no longer being generated but
	// we're still waiting for the currently playing audio chunk to
	// finish playing.
	Pa_Sleep(10000000);

	g_synth = nullptr;
	return Nil<Error>();
}

int main(int argc, char** argv) {
	std::string midi_in_name = "A-PRO 1";
	// for (int i = 0; i < argc; i++) {
	// 	if (std::string(argv[i]) == "--midi_in" && i+1 < argc) {
	// 		midi_in_name = argv[i+1];
	// 	}
	// }
	// std::cout << "midi_in: " << midi_in_name << std::endl;
	// return 0;
	
	Optional<Error> maybe_err = run(midi_in_name);
	if (!maybe_err.is_nil()) {
		const auto& err = maybe_err.ValueOrDie();
		std::cout << "error: " << err.str() << std::endl;
	}
	return 0;
};
