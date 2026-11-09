#ifndef PLUGIN_STATE_H
#define PLUGIN_STATE_H
#include <readerwriterqueue.h>

#include "../api.hh"
#include "pitch.hh"

namespace {
using namespace moodycamel;

/**
 * @brief Plugin state container.
 * This class will be host to different types of SPSC queues and provide
 * getters.
 * Essentially this is host to communication between processor and UI threads.
 */
class State {
 public:
  State() { spectra = ReaderWriterQueue<FBuffer>(16); }

  // UI -> Processor
  std::atomic<pitch::Key> key{pitch::Key::C};

  // Processor -> UI
  ReaderWriterQueue<FBuffer> spectra;

  // Detected fundamental frequency (Hz) from the pitch tracker; 0 if none.
  std::atomic<unsigned> f0{0};
};
}  // namespace

#endif  // PLUGIN_STATE_H