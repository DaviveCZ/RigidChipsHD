#pragma once

#include <windows.h>
#include <mmeapi.h>
#include <mmsystem.h>

#define AUDIO (AudioManager::sInstance)

class AudioManager
{
public:
    AudioManager();
    virtual ~AudioManager();

    static AudioManager* sInstance;

    bool Initialize();
    void Destroy();

    UINT sendMIDIEvent(BYTE bStatus, BYTE bData1, BYTE bData2);

private:
    HMIDIOUT mHMidiOut;
};
