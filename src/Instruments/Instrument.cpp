#include "Instrument.hpp"

#include "../Utils/Utils.hpp"

#include <algorithm>

void CppSynth::Instruments::Instrument::onEvent(const TimedEvent& tevent)
{

	switch (tevent.event.type) {
	case EventType::KEY_ON:
		m_CurrentEvents.push_back(tevent);
		break;

	case EventType::KEY_OFF:
		{
			auto correspondingKeyOn = std::find_if(
                m_CurrentEvents.begin(),
                m_CurrentEvents.end(),
                [&tevent](const TimedEvent& e) {
                    return e.event.type == EventType::KEY_ON && e.event.pitch == tevent.event.pitch;
                }
            );
			if (correspondingKeyOn == m_CurrentEvents.end()) {
				return;
			}
			*correspondingKeyOn = tevent;
		}
		break;
		
	default: break;
	}

}

void CppSynth::Instruments::Instrument::fillAudioBuffer(std::span<Sample> buffer, uint64_t bufferStartTimeSamples, const AudioSystem::AudioSystemSettings& audioSettings) noexcept
{
    for (const TimedEvent& tevent : m_CurrentEvents) {
        if (tevent.event.type == EventType::KEY_OFF) {
            continue;
        }

        auto firstSampleToFill = buffer.begin();
        auto lastSampleToFill = buffer.end(); // never changes

        // If positive (event occurred before the buffer started), indicates a phase of the signal about to be generated.
        // If negative (event occurred after the buffer started), indicates that the event starts somewhere in the middle of the buffer.
        int64_t timeBetweenEventAndBufferStartSamples = static_cast<int64_t>(bufferStartTimeSamples - tevent.timeSamples);
        uint64_t startingPhase = timeBetweenEventAndBufferStartSamples;

        if (timeBetweenEventAndBufferStartSamples < 0) {
            startingPhase = 0;
            firstSampleToFill = std::next(firstSampleToFill, -timeBetweenEventAndBufferStartSamples);
        }

        // Prepare section of buffer this note is supposed to render to
        std::span<Sample> noteDataBufferSpan(firstSampleToFill, lastSampleToFill);

        NoteRenderParams params = {
            .timedEvent = tevent,
            .generatedSignalStartOffset = startingPhase,
            .amplitude = 0.03,
            .pitch = A4 * pow(2, tevent.event.pitch / 12.0),
            .sampleRate = audioSettings.sampleRate,
            .stereo = audioSettings.stereo,
        };

        renderForEvent(noteDataBufferSpan, params);
    }
}
