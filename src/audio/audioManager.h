#pragma once

#include "midiUtil.h"

#define AUDIO (AudioManager::sInstance)

class AudioManager
{
public:
    AudioManager();
    virtual ~AudioManager();

    static AudioManager* sInstance;

    bool Initialize();
    void Destroy();

    void midiSetNote(BYTE bChannel, BYTE bNote, BYTE strength, bool on = true);
    void midiSetInstrument(BYTE bChannel, BYTE bPatch);
    void midiSetVolume(BYTE bChannel, BYTE bVolume);

private:
    HMIDIOUT mHMidiOut;

    UINT sendMIDIEvent(BYTE bStatus, BYTE bData1, BYTE bData2);

    void midiControlChange(BYTE bChannel, BYTE bController, BYTE bValue);
};
