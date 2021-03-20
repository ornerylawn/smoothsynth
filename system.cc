#include "system.h"

#include <cstring>

namespace {
Optional<Error> PortError(PaError err) {
  if (err == paNoError) {
    return Nil<Error>();
  }
  return AsOptional(Error(Pa_GetErrorText(err)));
}

Optional<Error> PortError(PmError err) {
  if (err == pmNoError) {
    return Nil<Error>();
  }
  return AsOptional(Error(Pm_GetErrorText(err)));
}

int AudioDriverCallback(const void* in_ptr, void* out_ptr,
                        unsigned long frame_count,
                        const PaStreamCallbackTimeInfo* time_info,
                        PaStreamCallbackFlags status_flags, void* this_ptr) {
  auto* sys = reinterpret_cast<System*>(this_ptr);
  sys->AudioDriverCallback(reinterpret_cast<const float*>(in_ptr),
                           reinterpret_cast<float*>(out_ptr), frame_count);
  return paContinue;
}

Optional<Error> GetMidiInDeviceIndex(const std::string& device_name,
                                     int* device_index) {
  int index = -1;
  int n = Pm_CountDevices();
  for (int i = 0; i < n; i++) {
    auto* info = Pm_GetDeviceInfo(i);
    if (info->input && std::string(info->name) == device_name) {
      index = i;
      break;
    }
  }
  if (index == -1) {
    return AsOptional(Error("MIDI input device not found"));
  }
  *device_index = index;
  return Nil<Error>();
}
}  // namespace

Optional<Error> System::Start() {
  RETURN_IF_ERROR(PortError(Pa_Initialize()));
  audio_initialized_ = true;

  RETURN_IF_ERROR(PortError(
      Pa_OpenDefaultStream(&audio_out_stream_, 0, 2, paFloat32, sample_rate_,
                           frames_per_chunk_, ::AudioDriverCallback, this)));
  audio_out_stream_opened_ = true;

  RETURN_IF_ERROR(PortError(Pa_StartStream(audio_out_stream_)));
  audio_out_stream_started_ = true;

  std::cout << "Audio initialized." << std::endl;

  RETURN_IF_ERROR(PortError(Pm_Initialize()));
  midi_initialized_ = true;

  std::cout << "midi_in_names_.size(): " << midi_in_names_.size() << std::endl;

  for (int i = 0; i < midi_in_names_.size(); ++i) {
    int midi_in_device_index;
    RETURN_IF_ERROR(GetMidiInDeviceIndex(midi_in_names_[i], &midi_in_device_index));

    RETURN_IF_ERROR(
        PortError(Pm_OpenInput(&midi_in_streams_[i], midi_in_device_index, nullptr,
                               midi_in_.capacity(), nullptr, nullptr)));
    CHECK(midi_in_streams_[i] != nullptr);
    midi_in_streams_opened_[i] = true;

    std::cout << "MIDI initialized (" << midi_in_names_[i] << ")." << std::endl;
  }

  return Nil<Error>();
}

void System::AudioDriverCallback(const float* in, float* out, int frame_count) {
  CHECK(out != nullptr);
  CHECK(frame_count <= frames_per_chunk_);

  midi_in_.set_tx(false);
  int midi_count = 0;
  for (int i = 0; i < midi_in_streams_.size(); ++i) {
    CHECK(midi_in_streams_[i] != nullptr);
    if (midi_in_streams_opened_[i] && Pm_Poll(midi_in_streams_[i])) {
      midi_count +=
          Pm_Read(midi_in_streams_[i], midi_in_.write_ptr() + midi_count,
                  midi_in_.capacity() - midi_count);
    }
  }
  midi_in_.set_size(midi_count);
  midi_in_.set_tx(true);

  callback_(frame_count);
  CHECK(stereo_out_->tx());

  const float* stereo_out = stereo_out_->read_ptr();
  int stereo_out_size = stereo_out_->size();
  CHECK(stereo_out_size == frame_count * 2);
  memcpy(out, stereo_out, sizeof(float) * stereo_out_size);
}

System::~System() {
  if (audio_out_stream_started_) {
    Pa_StopStream(audio_out_stream_);
  }
  if (audio_out_stream_opened_) {
    Pa_CloseStream(audio_out_stream_);
  }
  if (audio_initialized_) {
    Pa_Terminate();
  }
  for (int i = 0; i < midi_in_streams_.size(); ++i) {
    if (midi_in_streams_opened_[i]) {
      Pm_Close(midi_in_streams_[i]);
    }
  }
  if (midi_initialized_) {
    Pm_Terminate();
  }
}
