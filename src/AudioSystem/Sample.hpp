#include "../Utils/Array.hpp"

#include <AL/al.h>
#include <cstdint>

// This file is a mess, that's not important right now


#define AUDIO_FORMAT AL_FORMAT_MONO16
//#define AUDIO_FORMAT AL_FORMAT_MONO8

#if AUDIO_FORMAT == AL_FORMAT_MONO8 || AUDIO_FORMAT == AL_FORMAT_STEREO8
#error "8-bit autio format works with some bugs"
using Sample = uint8_t;
#define SAMPLE_MAX_VALUE UINT8_MAX
#define SAMPLE_MIN_VALUE 0
// A type only to be used in the SAMPLE_EQUILLIBRIUM macro.
// The smallest type that can be used to accurately compute the macro.
// The computation happens at compile-time anyway, but just for safety...
#define _SAMPLE_HIGHER_TYPE int16_t
#elif AUDIO_FORMAT == AL_FORMAT_MONO16 || AUDIO_FORMAT == AL_FORMAT_STEREO16
using Sample = int16_t;
#define SAMPLE_MAX_VALUE INT16_MAX
#define SAMPLE_MIN_VALUE INT16_MIN
// A type only to be used in the SAMPLE_EQUILLIBRIUM macro.
// The smallest type that can be used to accurately compute the macro.
// The computation happens at compile-time anyway, but just for safety.
#define _SAMPLE_HIGHER_TYPE int32_t
#endif
#define SAMPLE_EQUILIBRIUM (static_cast<Sample>((static_cast<_SAMPLE_HIGHER_TYPE>(SAMPLE_MAX_VALUE) + static_cast<_SAMPLE_HIGHER_TYPE>(SAMPLE_MIN_VALUE)) / 2))

namespace CppSynth {

	/// <summary>
	/// A buffer of samples. "A" stands for "Audio".
	/// </summary>
	using ABuffer = Array<Sample>;

}
