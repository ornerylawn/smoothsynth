#ifndef SYNTH_H_
#define SYNTH_H_

#include <portmidi.h>

#include "base.h"
#include "node.h"

class Synth : public Node {
public:
	virtual ~Synth() {}
	virtual void set_maybe_midi_in(const Optional<ArrayView<PmEvent>>* maybe_midi_in) = 0;
	virtual const Optional<ArrayView<float>>* maybe_stereo_out() const = 0;
};

#endif  // SYNTH_H_
