#ifndef SEQUENCER_H_
#define SEQUENCER_H_

#include <portmidi.h>

#include "base.h"
#include "node.h"
#include "chunk.h"

class Sequencer : public Node {
public:
	Sequencer(int sample_rate, int frames_per_chunk, int voices);
	virtual ~Sequencer() {}

	void set_midi_in(const ChunkTx<PmEvent>* midi_in) {
		midi_in_ = midi_in;
	}

	const ChunkTx<float>* frequency_outs(int i) const {
		return &frequency_outs_[i];
	}

	const ChunkTx<float>* trigger_outs(int i) const {
		return &trigger_outs_[i];
	}

	void StopTx() override {
		for (int i = 0; i < voices_; i++) {
			frequency_outs_[i].Stop();
			trigger_outs_[i].Stop();
		}
	}

	bool RxAvailable() const override {
		return midi_in_ != nullptr && midi_in_->available();
	}

	void Compute(int frame_count) override;

	void StartTx() override {
		for (int i = 0; i < voices_; i++) {
			frequency_outs_[i].Start();
			trigger_outs_[i].Start();
		}
	}
	
private:
	void TurnNoteOn(int note);
	void TurnNoteOff(int note);
	void PrintList();

	int voices_;

  // When a note is turned on, we need to use one of the unused
  // voices, or recycle a voice in use. If we're picking a voice in
  // use, we obviously want the oldest one so as to not interrupt the
  // note that was just played. If we're picking an unused voice, we
  // also want the oldest unused because there is still a release time
  // after the voice is triggered off, which we'd like to not
  // interrupt.

	// Getting the oldest voice, begs for a ring buffer. However, a
	// voice that is in use could turn off at any time, which would mean
	// that we'd need to remove from the middle of the list. One option
	// would be to nil out items in the list, but this means that our
	// number of items could continue to grow (imagine if the first note
	// played is not released). This means a linked list would be
	// better, as we could remove the item from the middle easily.

	// Since we cannot allocate memory on the audio thread, we'll just
	// have an array of nodes, one for each voice. And we just need to
	// keep the order: unused voices first oldest to newest, followed by
	// voices in use oldest to newest.

	struct Voice {
		Optional<Voice*> next, prev;

		int index;
		bool on;
		int note;
	};

	// TODO: use three sentinel nodes?
	FixedArray<Voice> voice_list_;
	Voice* front_;
	Voice* back_;
	Optional<Voice*> last_unused_;
	
	FixedArray<Optional<Voice*>> voice_by_note_;


	// Inputs.
	const ChunkTx<PmEvent>* midi_in_;

	// Outputs.
	FixedArray<ChunkTx<float>> frequency_outs_;
	FixedArray<ChunkTx<float>> trigger_outs_;
};

#endif  // SEQUENCER_H_
