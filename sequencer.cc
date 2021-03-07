#include "sequencer.h"

#include <cmath>

Sequencer::Sequencer(int sample_rate, int frames_per_chunk, int voices)
    : voices_(voices),
      voice_list_(voices),
      voice_by_note_(128),
      midi_in_(nullptr),
      frequency_outs_(voices, ChunkTx<float>(frames_per_chunk)),
      trigger_outs_(voices, ChunkTx<float>(frames_per_chunk)) {
  for (int i = 0; i < voices; i++) {
    auto& voice = voice_list_[i];
    voice.index = i;
    voice.on = false;
    voice.note = 69;

    if (i - 1 >= 0) {
      voice.prev = &voice_list_[i - 1];
    }
    if (i + 1 < voices) {
      voice.next = &voice_list_[i + 1];
    }
  }
  front_sentinel_.prev = nullptr;
  front_sentinel_.next = &voice_list_[0];
  back_sentinel_.prev = &voice_list_[voices - 1];
  back_sentinel_.next = nullptr;
  insert_point_ = &voice_list_[voices - 1];  // last unused voice
}

bool Sequencer::Rx() { return midi_in_ != nullptr && midi_in_->tx(); }

void Sequencer::ComputeAndStartTx(int frame_count) {
  const PmEvent* midi_in = midi_in_->read_ptr();
  int midi_in_size = midi_in_->size();

  for (int i = 0; i < voices_; i++) {
    frequency_outs_[i].set_size(frame_count);
    trigger_outs_[i].set_size(1);
    *(trigger_outs_[i].write_ptr()) = 0.0f;
  }

  for (int i = 0; i < midi_in_size; i++) {
    const auto& event = midi_in[i];
    switch (Pm_MessageStatus(event.message)) {
      case 128:
        TurnNoteOff(Pm_MessageData1(event.message));
        break;
      case 144:
        TurnNoteOn(Pm_MessageData1(event.message));
        break;
      default:
        break;
    }
  }

  // TODO: would be nice if we could have transitions not synced to
  // chunk boundaries.
  for (int i = 0; i < voices_; i++) {
    int note = voice_list_[i].note;
    float f = 440.0 * std::pow(2, (note - 69) / 12.0);
    float* frequency_out = frequency_outs_[i].write_ptr();
    for (int j = 0; j < frame_count; j++) {
      frequency_out[j] = f;
    }
  }

  for (int i = 0; i < voices_; i++) {
    frequency_outs_[i].set_tx(true);
    trigger_outs_[i].set_tx(true);
  }
}

void Sequencer::StopTx() override {
  for (int i = 0; i < voices_; i++) {
    frequency_outs_[i].set_tx(false);
    trigger_outs_[i].set_tx(false);
  }
}

void Sequencer::TurnNoteOn(int note) {
  // The first item in the list is always the one we should use. It is
  // either a non-last unused voice, the last unused voice (if there's
  // only one unused voice left), or the oldest voice in use (if there
  // are no unused voices left).

  // Remove from front.
  auto* left = &front_sentinel_;
  auto* voice = left->next;
  auto* right = voice->next;
  left->next = right;
  right->prev = left;
  voice->prev = nullptr;
  voice->next = nullptr;

  if (insert_point_ == voice) {
    insert_point_ = &front_sentinel_;
  }

  // If the voice is already on, then it should be unassociated from
  // its current note. If the voice is off, then we need to turn it
  // on. In either case we need to associate with the new note and
  // trigger the voice.
  if (voice->on) {
    voice_by_note_[voice->note] = nullptr;
  }
  voice->on = true;
  voice->note = note;
  voice_by_note_[note] = voice;
  *(trigger_outs_[voice->index].write_ptr()) = 1.0f;

  // Move to end.
  right = &back_sentinel_;
  left = right->prev;
  left->next = voice;
  voice->prev = left;
  voice->next = right;
  right->prev = voice;
}

void Sequencer::TurnNoteOff(int note) {
  Voice* voice = voice_by_note_[note];
  if (voice == nullptr) {
    return;  // note isn't on
  }

  // Remove from list.
  voice->prev->next = voice->next;
  voice->next->prev = voice->prev;
  voice->prev = nullptr;
  voice->next = nullptr;

  // Turn the note off (we need to leave voice->note for ADSR release).
  voice_by_note_[note] = nullptr;
  voice->on = false;
  *(trigger_outs_[voice->index].write_ptr()) = -1.0f;

  // Insert as new insert_point_ to make sure that unused voices are
  // always used before used ones, and that the newest unused voice
  // has a chance to ADSR release.
  auto* left = insert_point_;
  auto* right = left->next;
  right->prev = voice;
  voice->next = right;
  voice->prev = left;
  left->next = voice;
  insert_point_ = voice;
}
