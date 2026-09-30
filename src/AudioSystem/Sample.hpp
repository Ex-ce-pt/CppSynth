#ifndef CPPSYNTH_AUDIOSYSTEM_SAMPLE_HPP
#define CPPSYNTH_AUDIOSYSTEM_SAMPLE_HPP

#include <AL/al.h>
#include <cstdint>
#include <limits>

// TODO: support various audio formats (when the time comes)

#define AUDIO_FORMAT AL_FORMAT_MONO16
//#define AUDIO_FORMAT AL_FORMAT_MONO8

namespace CppSynth {

#if AUDIO_FORMAT == AL_FORMAT_MONO8 || AUDIO_FORMAT == AL_FORMAT_STEREO8
#error "8-bit autio format works with some bugs"

	using NativeSampleT = uint8_t; // TODO: confirm the type
	inline constexpr NativeSampleT NATIVE_SAMPLE_MAX = std::numeric_limits<NativeSampleT>::max();
	inline constexpr NativeSampleT NATIVE_SAMPLE_MIN = std::numeric_limits<NativeSampleT>::min();
	inline constexpr NativeSampleT NATIVE_SAMPLE_EQUILIBRIUM = (NATIVE_SAMPLE_MAX + NATIVE_SAMPLE_MIN) / 2;
	inline constexpr NativeSampleT NATIVE_SAMPLE_AMPLITUDE = NATIVE_SAMPLE_MIN + NATIVE_SAMPLE_EQUILIBRIUM;

#elif AUDIO_FORMAT == AL_FORMAT_MONO16 || AUDIO_FORMAT == AL_FORMAT_STEREO16

	using NativeSampleT = int16_t;
	inline constexpr NativeSampleT NATIVE_SAMPLE_MAX = std::numeric_limits<NativeSampleT>::max();
	inline constexpr NativeSampleT NATIVE_SAMPLE_MIN = std::numeric_limits<NativeSampleT>::min();
	inline constexpr NativeSampleT NATIVE_SAMPLE_EQUILIBRIUM = (NATIVE_SAMPLE_MAX + NATIVE_SAMPLE_MIN) / 2;
	inline constexpr NativeSampleT NATIVE_SAMPLE_AMPLITUDE = NATIVE_SAMPLE_MAX - NATIVE_SAMPLE_EQUILIBRIUM;

#endif

	/// <summary>
	/// A set type for the entire application.
	/// A sample is a double in the range from -1 to 1 ends included.
	/// </summary>
	using Sample = double;
	inline constexpr Sample SAMPLE_MAX = 1.0;
	inline constexpr Sample SAMPLE_MIN = -1.0;
	inline constexpr Sample SAMPLE_EQUILIBRIUM = 0.0;
	inline constexpr Sample SAMPLE_AMPLITUDE = 1.0;

}

#endif /* CPPSYNTH_AUDIOSYSTEM_SAMPLE_HPP */
