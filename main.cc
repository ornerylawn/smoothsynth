#include <vector>

#include "base.h"
#include "system.h"
#include "synth.h"
#include <ncurses.h>
#include <signal.h>

Synth* global_synth = nullptr;

void AudioDriverCallback(int frame_count) {
	CHECK(global_synth != nullptr);
	global_synth->StopTx();
	CHECK(global_synth->RxAvailable());
	global_synth->Compute(frame_count);
	global_synth->StartTx();
	CHECK(global_synth->stereo_out()->available());
}

Optional<Error> PowerOn(const std::string& midi_in_name,
												int midi_in_buffer_size,
												int sample_rate,
												int frames_per_chunk,
												int voices) {
	CHECK(!midi_in_name.empty());
	System sys(midi_in_buffer_size, midi_in_name, sample_rate, frames_per_chunk,
						 AudioDriverCallback);
	Synth synth(sample_rate, frames_per_chunk, voices);
	synth.set_midi_in(sys.midi_in());
	sys.set_stereo_out(synth.stereo_out());
	global_synth = &synth;
	RETURN_IF_ERROR(sys.Start());

	// TODO: Block here and wait for stop control. There should be a
	// lameduck period where audio is no longer being generated but
	// we're still waiting for the currently playing audio chunk to
	// finish playing.
	Pa_Sleep(10000000);

	global_synth = nullptr;
	return Nil<Error>();
}

Optional<Error> ListMidiDevicesCommand() {
	int n = Pm_CountDevices();
	for (int i = 0; i < n; i++) {
		auto* info = Pm_GetDeviceInfo(i);
		if (info->input) {
			std::cout << info->name << "\n";
		}
	}
	std::cout << std::endl;
	return Nil<Error>();
}

const char* power_on_usage =
	"usage: smoothsynth power_on <args>\n"
	"\n"
	"args\n"
	"  --midi_in <name of input device>\n";

Optional<Error> PowerOnCommand(const std::vector<std::string>& args) {
	int midi_in_buffer_size = 32;
	int sample_rate = 44100;
	int frames_per_chunk = 1024;
	int voices = 8;
	std::string midi_in_name;

	for (int i = 0; i < args.size(); i++) {
		if (args[i] == "--help") {
			return AsOptional(Error(power_on_usage));
		} else if (args[i] == "--midi_in") {
			if (i+1 == args.size()) {
				return AsOptional(Error(power_on_usage));
			}
			midi_in_name = args[i+1];
		}
	}

	if (midi_in_name.empty()) {
		return AsOptional(Error(power_on_usage));
	}
	
	return PowerOn(midi_in_name, midi_in_buffer_size, sample_rate,
								 frames_per_chunk, voices);
}

const char* smoothsynth_usage =
	"usage: smoothsynth <command> <args>\n"
	"\n"
	"commands\n"
	"  list_midi    lists midi device names\n"
	"  power_on     turns the synth on\n";

// Optional<Error> smoothsynth(int argc, char** argv) {
// 	if (argc < 2) {
// 		return AsOptional(Error(smoothsynth_usage));
// 	}
// 	std::string command(argv[1]);
// 	std::vector<std::string> args;
// 	if (argc >= 3) {
// 		args.assign(&argv[2], &argv[argc]);
// 	}
// 	if (command == "list_midi") {
// 		return ListMidiDevicesCommand();
// 	} else if (command == "power_on") {
// 		return PowerOnCommand(args);
// 	} else {
// 		return AsOptional(Error(smoothsynth_usage));
// 	}
// }

// TODO: make the synth a repl, with commands to modify midi
// connection and mapping, that way we can assign say a control
// voltage to the frequency of a vco and control it with a physical
// knob.

std::vector<std::string> Tokenize(std::string s) {
  std::vector<std::string> tokens;
  int lo = 0;
  for (;;) {
    while (lo < s.size() && s[lo] == ' ') { lo++; }
    if (lo == s.size()) { break; }
    int hi = lo+1;
    while (hi < s.size() && s[hi] != ' ') { hi++; }
    tokens.push_back(s.substr(lo, hi-lo));
    lo = hi;
  }
  return tokens;
}

#define CTRL(c) ((c) & 037)

void Finish(int sig) {
  endwin();
  exit(0);
}

class Repl {
public:
  Repl() {
    signal(SIGINT, Finish);
    initscr();
    keypad(stdscr, TRUE);
    cbreak();
    noecho();
    clear();

    cursor_index_ = 0;
    x_scroll_ = 0;
    last_c_ = -1;
  }
  ~Repl() {
    Finish(-1);
  }

  void Run() {
    for (;;) {
      Render();
      last_c_ = getch();
      ProcessKey(last_c_);
      
    }
  }

private:
  
  void Render() {
    erase();
    getmaxyx(stdscr, h_, w_);

    RenderHeader();
    RenderHistoricalLines();
    RenderCurrentLine();

    move(h_-1, 2+cursor_index_-x_scroll_);

    refresh();
  }

  void RenderHeader() {
    int x = 2+cursor_index_-x_scroll_;
    int y = h_-1;
    mvprintw(0, 0, "Smoothie (h = %d, w = %d, x = %d, y = %d, c = %d)", h_, w_, x, y, last_c_);
  }

  void RenderHistoricalLines() {
    int h = h_ - 2;  // minus header and current line
    int hi = historical_lines_.size();
    int lo = std::max(0, hi - h);
    int start_y = 1 + h - (hi-lo);
    for (int i = lo; i < hi; i++) {
      mvaddstr(start_y+i-lo, 0, historical_lines_[i].c_str());
    }
  }

  void RenderCurrentLine() {
    mvprintw(h_-1, 0, "> %s", current_line_.c_str());
  }

  void ProcessKey(int c) {
    switch (c) {
    case 410:  // screen resize
      break;
    case KEY_LEFT:
      MoveCursorLeft();
      break;
    case KEY_RIGHT:
      MoveCursorRight();
      break;
    case KEY_BACKSPACE:
      if (cursor_index_ != 0) {
        current_line_.erase(cursor_index_-1, 1);
        cursor_index_--;
      }
      break;
    case CTRL('a'):
      cursor_index_ = 0;
      break;
    case CTRL('e'):
      cursor_index_ = current_line_.size();
      break;
    case CTRL('b'):
      MoveCursorLeft();
      break;
    case CTRL('f'):
      MoveCursorRight();
      break;
    case CTRL('k'):
      current_line_.erase(cursor_index_);
      break;
    case '\n':
      historical_lines_.push_back(current_line_);
      current_line_.erase();
      cursor_index_ = 0;
      ProcessCommand(historical_lines_.back());
      break;
    default:
      current_line_.insert(cursor_index_, 1, (char)c);
      cursor_index_++;
      break;
    }
  }

  void MoveCursorRight() {
    cursor_index_ = std::min((int)current_line_.size(), cursor_index_+1);
  }

  void MoveCursorLeft() {
    cursor_index_ = std::max(0, cursor_index_-1);
  }

  void ProcessCommand(const std::string& cmd) {
    if (cmd == "list_midi") {
      ListMidiDevicesCommand();
    } else if (cmd == "power") {
      PowerOnCommand({"--midi_in", "A-PRO 1"});
    } else {
    }
  }

  int h_, w_;

  std::vector<std::string> historical_lines_;
  std::string current_line_;
  int cursor_index_;
  int x_scroll_;
  int last_c_;
};

Optional<Error> smoothsynth(int argc, char** argv) {
  Repl repl;
  repl.Run();
  return Nil<Error>();
}

int main(int argc, char** argv) {
	auto maybe_err = smoothsynth(argc, argv);
	if (!maybe_err.is_nil()) {
		const auto& err = maybe_err.ValueOrDie();
		std::cout << "error: " << err.str() << std::endl;
	}
	return 0;
};
