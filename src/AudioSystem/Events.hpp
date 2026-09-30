#ifndef CPPSYNTH_AUDIOSYSTEM_EVENTS_HPP
#define CPPSYNTH_AUDIOSYSTEM_EVENTS_HPP

#include <cstdint>

namespace CppSynth {

	enum class EventType {
		KEY_ON,
		KEY_OFF
	};

	struct Event {
		EventType type;
		double pitch;
	};

	struct TimedEvent {
		Event event;
		uint64_t timeSamples;
	};

}

#endif /* CPPSYNTH_AUDIOSYSTEM_EVENTS_HPP */
