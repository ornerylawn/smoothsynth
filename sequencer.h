#ifndef SEQUENCER_H_
#define SEQUENCER_H_

#include <portmidi.h>

#include "base.h"
#include "chunk.h"
#include "node.h"

class Sequencer : public Node {
 public:
  Sequencer(int sample_rate, int frames_per_chunk, int voices);
  virtual ~Sequencer() {}

  void set_midi_in(const ChunkTx<PmEvent>* midi_in) { midi_in_ = midi_in; }

  const ChunkTx<float>* frequency_cv_outs(int i) const {
    return &frequency_cv_outs_[i];
  }

  const ChunkTx<float>* trigger_outs(int i) const { return &trigger_outs_[i]; }

  float cutoff() { return cutoff_; }

  bool Rx() const override;
  void ComputeAndStartTx(int frame_count) override;
  void StopTx() override;

 private:
  void TurnNoteOn(int note);
  void TurnNoteOff(int note);

  const ChunkTx<PmEvent>* midi_in_;
  FixedArray<ChunkTx<float>> frequency_cv_outs_;
  FixedArray<ChunkTx<float>> trigger_outs_;
  float cutoff_;

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
    Voice* next;
    Voice* prev;
    int index;
    bool on;
    int note;
  };

  int voices_;
  FixedArray<Voice> voice_list_;
  Voice front_sentinel_;
  Voice back_sentinel_;
  Voice* insert_point_;

  FixedArray<Voice*> voice_by_note_;
};

#endif  // SEQUENCER_H_
