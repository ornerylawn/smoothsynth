#include "sequencer.h"

#include <cmath>
#include <sstream>

Sequencer::Sequencer(int sample_rate, int frames_per_chunk, int voices)
	: voices_(voices),
		frequency_outs_(voices, FixedArray<float>(frames_per_chunk)),
		maybe_frequency_outs_(voices),
		trigger_outs_(voices, FixedArray<float>(frames_per_chunk)),
		maybe_trigger_outs_(voices),
		voice_list_(voices),
		voice_by_note_(128) {
	for (int i = 0; i < voice_list_.size(); i++) {
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
	if (voices_ > 0) {
		CHECK(frame_count <= frequency_outs_[0].size());
	}

	ArrayView<PmEvent> midi_in = maybe_midi_in_->ValueOrDie();

	for (int voice = 0; voice < voices_; voice++) {
		trigger_outs_[voice][0] = 0.0f;
	}
	
	for (int i = 0; i < midi_in.size(); i++) {
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

	for (int voice = 0; voice < voices_; voice++) {
		int note = voice_list_[voice].note;
		float f = 440.0 * std::pow(2, (note-69)/12.0);
		FixedArray<float>& frequency_out = frequency_outs_[voice];
		for (int i = 0; i < frame_count; i++) {
			frequency_out[i] = f;
		}
		maybe_frequency_outs_[voice] = AsOptional(ArrayView<float>(&frequency_outs_[voice], 0, frame_count));
		maybe_trigger_outs_[voice] = AsOptional(ArrayView<float>(&trigger_outs_[voice], 0, frame_count));
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
	trigger_outs_[voice->index][0] = 1.0f;

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
	trigger_outs_[voice->index][0] = -1.0f;

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
	for (int i = 0; i < voice_list_.size(); i++) {
		const Voice* v = &voice_list_[i];
		if (v->on) {
			if (v->note < 10) {
				std::cout << "[" << v->index << ":0" << v->note << "]";
			} else {
				std::cout << "[" << v->index << ":" << v->note << "]";
			}
		} else {
			//std::cout << "[" << v->index << ":__]";
		}
	}
	std::cout << std::endl;
}
