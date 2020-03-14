#include "sequencer.h"

#include <cmath>
#include <sstream>

Sequencer::Sequencer(int sample_rate, int frames_per_chunk, int voices)
	: voices_(voices),
		voice_list_(voices),
		voice_by_note_(128),
		midi_in_(nullptr),
		frequency_outs_(voices, ChunkTx<float>(frames_per_chunk)),
		trigger_outs_(voices, ChunkTx<float>(frames_per_chunk)) {
	for (int i = 0; i < voices_; i++) {
		Voice* voice = &voice_list_[i];
		voice->index = i;
		voice->on = false;
		voice->note = 69;

		if (i+1 < voice_list_.size()) {
			voice->next = AsOptional(&voice_list_[i+1]);
		}
		if (i-1 >= 0) {
			voice->prev = AsOptional(&voice_list_[i-1]);
		}
	}
	front_ = &voice_list_[0];
	back_ = &voice_list_[voice_list_.size()-1];
	last_unused_ = AsOptional(back_);

	PrintList();
}

void Sequencer::Compute(int frame_count) {
	CHECK(midi_in_ != nullptr);
	const PmEvent* midi_in = midi_in_->read_ptr();
	int midi_in_size = midi_in_->size();

	for (int i = 0; i < voices_; i++) {
		frequency_outs_[i].set_size(frame_count);
		trigger_outs_[i].set_size(1);
		*(trigger_outs_[i].write_ptr()) = 0.0f;
	}
	
	for (int i = 0; i < midi_in_size; i++) {
		PmEvent event = midi_in[i];
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
		float f = 440.0 * std::pow(2, (note-69)/12.0);
		float* frequency_out = frequency_outs_[i].write_ptr();
		for (int j = 0; j < frame_count; j++) {
			frequency_out[j] = f;
		}
	}
}

void Sequencer::TurnNoteOn(int note) {
	// The first item in the list is always the one we should use. It is
	// either a non-last unused voice, the last unused voice, or the
	// oldest voice in use.

	// Remove from front.
	Voice* voice = front_;
	if (voices_ > 1) {
		front_ = voice->next.ValueOrDie();
		front_->prev = Nil<Voice*>();
		voice->next = Nil<Voice*>();
	}
	if (!last_unused_.is_nil() && last_unused_.ValueOrDie() == voice) {
		last_unused_ = Nil<Voice*>();
	}
	
	// Turn the note on.
	if (voice->on) {
		voice_by_note_[voice->note] = Nil<Voice*>();
	} else {
		voice->on = true;
	}
	voice->note = note;
	voice_by_note_[note] = AsOptional(voice);
	*(trigger_outs_[voice->index].write_ptr()) = 1.0f;

	// Move to end.
	if (voices_ > 1) {
		voice->prev = AsOptional(back_);
		back_->next = AsOptional(voice);
		back_ = voice;
	}

	PrintList();
}

void Sequencer::TurnNoteOff(int note) {
	Optional<Voice*> maybe_voice = voice_by_note_[note];
	if (maybe_voice.is_nil()) {
		return;  // note isn't on
	}

	// We need to turn the voice off and move it to last_unused_->next,
	// and make it the last_unused_.

	// Remove from list.
	Voice* voice = maybe_voice.ValueOrDie();
	if (!voice->prev.is_nil()) {
		voice->prev.ValueOrDie()->next = voice->next;
	} else {
		// Front.
		if (voices_ > 1) {
			front_ = voice->next.ValueOrDie();
		}
	}
	if (!voice->next.is_nil()) {
		voice->next.ValueOrDie()->prev = voice->prev;
	} else {
		// Back.
		if (voices_ > 1) {
			back_ = voice->prev.ValueOrDie();
		}
	}
	voice->prev = Nil<Voice*>();
	voice->next = Nil<Voice*>();

	// Turn the note off (we need to leave voice->note for ADSR release).
	voice_by_note_[note] = Nil<Voice*>();
	voice->on = false;
	*(trigger_outs_[voice->index].write_ptr()) = -1.0f;

	// Insert as last_unused_.
	if (last_unused_.is_nil()) {
		// Insert at front as the only unused voice.
		if (voices_ > 1) {
			Voice* first_used = front_;
			first_used->prev = AsOptional(voice);
			voice->next = AsOptional(first_used);
			front_ = voice;
		}
	} else {
		Voice* last_unused = last_unused_.ValueOrDie();
		if (last_unused->next.is_nil()) {
			voice->prev = AsOptional(back_);
			back_->next = AsOptional(voice);
			back_ = voice;
		} else {
			Voice* first_used = last_unused->next.ValueOrDie();
			first_used->prev = AsOptional(voice);
			last_unused->next = AsOptional(voice);
			voice->next = AsOptional(first_used);
			voice->prev = AsOptional(last_unused);
		}
	}
	last_unused_ = AsOptional(voice);

	PrintList();
}

std::string itoa(int k) {
	std::stringstream ss;
	ss << k;
	return ss.str();
}

void Sequencer::PrintList() {
	// for (int i = 0; i < voice_list_.size(); i++) {
	// 	const Voice* v = &voice_list_[i];
	// 	if (v->on) {
	// 		if (v->note < 10) {
	// 			std::cout << "[" << v->index << ":0" << v->note << "]";
	// 		} else {
	// 			std::cout << "[" << v->index << ":" << v->note << "]";
	// 		}
	// 	} else {
	// 		//std::cout << "[" << v->index << ":__]";
	// 	}
	// }
	// std::cout << std::endl;
}
