CC=g++
CFLAGS=-std=c++11 -lportaudio -lportmidi
INLINEFILES = node.h synth.h chunk.h
HFILES = adsr.h base.h mono_to_stereo.h port_system.h sequencer.h smooth_synth.h vco.h lowpass.h
OFILES = adsr.o base.o mono_to_stereo.o port_system.o sequencer.o smooth_synth.o vco.o lowpass.o

%.o: %.cc $(HFILES) $(INLINEFILES)
	$(CC) -c -o $@ $< $(CFLAGS)

smooth_synth: main.cc $(OFILES)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o *~ smooth_synth
