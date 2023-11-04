#include "audioManager.h"

AudioManager* AudioManager::sInstance = new AudioManager();

AudioManager::AudioManager()
{
    mHMidiOut = nullptr;
}

AudioManager::~AudioManager()
{
    Destroy();
}

bool AudioManager::Initialize()
{
    Destroy();

    if(midiOutOpen(&mHMidiOut, (UINT) MIDI_MAPPER, NULL, 0, 0) == 0){
        // Engine Sound
        midiSetInstrument(MIDIChannel::Channel1, MIDIPatch::Helicopter);

        // Skid Sound
        midiSetInstrument(MIDIChannel::Channel2, MIDIPatch::Applause);
        midiSetVolume(MIDIChannel::Channel2, 0);
        midiSetNote(MIDIChannel::Channel2, MIDINotes::ASharp4, 63);
    }

    return true;
}

void AudioManager::Destroy()
{
    if (mHMidiOut)
    {
        midiOutClose(mHMidiOut);
        mHMidiOut=nullptr;
    }
}

UINT AudioManager::sendMIDIEvent(BYTE bStatus, BYTE bData1, BYTE bData2)
{
    // from: https://learn.microsoft.com/en-us/windows/win32/multimedia/using-midioutshortmsg-to-send-individual-midi-messages
    union {
        DWORD dwData;
        BYTE bData[4];
    } u;

    // Construct the MIDI message.

    u.bData[0] = bStatus;  // MIDI status byte
    u.bData[1] = bData1;   // first MIDI data byte
    u.bData[2] = bData2;   // second MIDI data byte
    u.bData[3] = 0;

    // Send the message.
    return midiOutShortMsg(mHMidiOut, u.dwData);
}

void AudioManager::midiSetNote(BYTE bChannel, BYTE bNote, BYTE strength, bool on)
{
    if (on)
        sendMIDIEvent(MIDIEvent::NoteOn | bChannel, bNote, strength);
    else
        sendMIDIEvent(MIDIEvent::NoteOff | bChannel, bNote, strength);
}

void AudioManager::midiControlChange(BYTE bChannel, BYTE bController, BYTE bValue)
{
    sendMIDIEvent(MIDIEvent::ControlChange | bChannel, bController, bValue);
}

void AudioManager::midiSetInstrument(BYTE bChannel, BYTE bPatch)
{
    sendMIDIEvent(MIDIEvent::ProgramChange | bChannel, bPatch, 0);
}

void AudioManager::midiSetVolume(BYTE bChannel, BYTE bVolume)
{
    midiControlChange(bChannel, 7, bVolume);
}
