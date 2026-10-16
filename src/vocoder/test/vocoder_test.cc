#include "../vocoder.hh"

#include <gtest/gtest.h>

#include "AudioFile.h"

TEST(VocoderTest, Constructor) { Vocoder voc(1, 2, 3); }