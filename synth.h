#ifndef SYNTH_H_
#define SYNTH_H_

#include <portmidi.h>

#include "base.h"
#include "node.h"
#include "chunk.h"

class Synth : public Node {
public:
	virtual ~Synth() {}
	virtual void set_midi_in(const ChunkTx<PmEvent>* midi_in) = 0;
	virtual const ChunkTx<float>* stereo_out() const = 0;
};

#endif  // SYNTH_H_
