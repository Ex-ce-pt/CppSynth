#include "Instrument.hpp"

#include <algorithm>

void CppSynth::Instruments::Instrument::onEvent(const TimedEvent& tevent)
{
	switch (tevent.Event.Type) {
	case EventType::KEY_ON:
		m_CurrentEvents.push_back(tevent);
		break;

	case EventType::KEY_OFF:
		{
			auto correspondingKeyOn = std::find_if(m_CurrentEvents.begin(), m_CurrentEvents.end(), [&tevent](const TimedEvent& e) { return e.Event.Type == EventType::KEY_ON && e.Event.Pitch == tevent.Event.Pitch; });
			if (correspondingKeyOn == m_CurrentEvents.end()) return;
			*correspondingKeyOn = tevent;
		}
		break;
		
	default: break;
	}
}
