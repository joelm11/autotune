#ifndef PLUGIN_STATE_H
#define PLUGIN_STATE_H
#include <readerwriterqueue.h>

#include "../api.hh"

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

  ReaderWriterQueue<FBuffer> spectra;
};
}  // namespace

#endif  // PLUGIN_STATE_H