CC=g++
CFLAGS=-std=c++17 -lportaudio -lportmidi -lncurses
INLINEFILES = node.h chunk.h
HFILES =  base.h system.h synth.h sequencer.h adsr.h vco.h unison.h vcf.h vca.h mixer.h
OFILES =  base.o system.o synth.o sequencer.o adsr.o vco.o unison.o vcf.o vca.o mixer.o

%.o: %.cc $(HFILES) $(INLINEFILES)
	$(CC) -c -o $@ $< $(CFLAGS)

smoothsynth: main.cc $(OFILES)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o *~ smoothsynth
