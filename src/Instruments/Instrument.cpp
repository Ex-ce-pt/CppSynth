#include "Instrument.hpp"

#include <algorithm>

void CppSynth::Instruments::Instrument::message(const Message& msg)
{
	switch (msg.type) {
	case MessageType::KEY_ON:
		messages.push_back(msg);
		break;

	case MessageType::KEY_OFF:
		{
			auto correspondingKeyOn = std::find_if(messages.begin(), messages.end(), [&msg](const Message& m) { return m.type == MessageType::KEY_ON && m.pitch == msg.pitch; });
			if (correspondingKeyOn == messages.end()) return;
			*correspondingKeyOn = msg;
		}
		break;
		
	default: break;
	}
}
