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

    long msg;
    if(midiOutOpen(&mHMidiOut, (UINT) MIDI_MAPPER, NULL, 0, 0) == 0){
        //GM system on
        //Change the tone (0xCn, tone, 0x00) n is the channel (0 to 0xF)
        sendMIDIEvent(0xC0, 0x7D, 0x00); // Note On
        sendMIDIEvent(0xC1, 0x7E, 0x00); // Note On
        sendMIDIEvent(0xB1, 0x07, 0x00); // Note On
        sendMIDIEvent(0x91, 70, 0x3f); // Note On
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
