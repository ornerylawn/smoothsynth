#ifndef PORT_SYSTEM_H_
#define PORT_SYSTEM_H_

#include <portaudio.h>
#include <portmidi.h>

#include <functional>
#include <vector>

#include "chunk.h"

class System {
 public:
  System(int midi_in_buffer_size, const std::vector<std::string>& midi_in_names,
         int sample_rate, int frames_per_chunk,
         std::function<void(int)> callback)
      : midi_in_(midi_in_buffer_size),
        stereo_out_(nullptr),
        midi_in_names_(midi_in_names),
        midi_in_streams_(midi_in_names.size()),
        midi_in_streams_opened_(midi_in_names.size()),
        sample_rate_(sample_rate),
        frames_per_chunk_(frames_per_chunk),
        callback_(callback) {
          for (int i = 0; i < midi_in_streams_.size(); ++i) {
            midi_in_streams_[i] = nullptr;
            midi_in_streams_opened_[i] = false;
          }
        }

  ~System();

  const ChunkTx<PmEvent>* midi_in() { return &midi_in_; }

  void set_stereo_out(const ChunkTx<float>* stereo_out) {
    stereo_out_ = stereo_out;
  }

  Optional<Error> Start();

  void AudioDriverCallback(const float* in, float* out, int frame_count);

 private:
  const std::vector<std::string>& midi_in_names_;
  int sample_rate_;
  int frames_per_chunk_;

  std::vector<PortMidiStream*> midi_in_streams_;
  PaStream* audio_out_stream_;

  bool midi_initialized_;
  std::vector<bool> midi_in_streams_opened_;
  bool audio_initialized_;
  bool audio_out_stream_opened_;
  bool audio_out_stream_started_;

  ChunkTx<PmEvent> midi_in_;
  const ChunkTx<float>* stereo_out_;

  std::function<void(int)> callback_;
};

#endif  // PORT_SYSTEM_H_
