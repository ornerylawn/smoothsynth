CC=g++
CFLAGS=-std=c++11 -lportaudio -lportmidi -lncurses
INLINEFILES = node.h chunk.h
HFILES = adsr.h base.h mono_to_stereo.h system.h sequencer.h synth.h vco.h lowpass.h mixer.h
OFILES = adsr.o base.o mono_to_stereo.o system.o sequencer.o synth.o vco.o lowpass.o mixer.o

%.o: %.cc $(HFILES) $(INLINEFILES)
	$(CC) -c -o $@ $< $(CFLAGS)

smoothsynth: main.cc $(OFILES)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o *~ smoothsynth
