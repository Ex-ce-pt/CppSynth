#include "Instrument.hpp"

#include <algorithm>

void CppSynth::Instruments::Instrument::onEvent(const TimedEvent& _event)
{
	switch (_event.e.type) {
	case EventType::KEY_ON:
		events.push_back(_event);
		break;

	case EventType::KEY_OFF:
		{
			auto correspondingKeyOn = std::find_if(events.begin(), events.end(), [&_event](const TimedEvent& _e) { return _e.e.type == EventType::KEY_ON && _e.e.pitch == _event.e.pitch; });
			if (correspondingKeyOn == events.end()) return;
			*correspondingKeyOn = _event;
		}
		break;
		
	default: break;
	}
}
