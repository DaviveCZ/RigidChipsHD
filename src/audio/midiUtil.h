#pragma once

#include <windows.h>
#include <mmeapi.h>
#include <mmsystem.h>

enum MIDIEvent : BYTE
{
    NoteOn = 0x90,
    NoteOff = 0x80,
    ControlChange = 0xB0,
    ProgramChange = 0xC0,
};

enum MIDIChannel : BYTE
{
    Channel1,
    Channel2,
    Channel3,
    Channel4,
    Channel5,
    Channel6,
    Channel7,
    Channel8,
    Channel9,
    Channel10_Percussion, // Channel 10 is reserved for percussion
    Channel11,
    Channel12,
    Channel13,
    Channel14,
    Channel15,
    Channel16,
};

enum MIDIPatch : BYTE
{
    // Piano
    AcousticGrandPiano = 0,
    BrightAcousticPiano = 1,
    ElectricGrandPiano = 2,
    HonkyTonkPiano = 3,
    ElectricPiano1 = 4,
    ElectricPiano2 = 5,
    Harpsichord = 6,
    Clavinet = 7,

    // Chromatic Percussion
    Celesta = 8,
    Glockenspiel = 9,
    MusicBox = 10,
    Vibraphone = 11,
    Marimba = 12,
    Xylophone = 13,
    TubularBells = 14,
    Dulcimer = 15,

    // Organ
    DrawbarOrgan = 16,
    PercussiveOrgan = 17,
    RockOrgan = 18,
    ChurchOrgan = 19,
    ReedOrgan = 20,
    Accordian = 21,
    Harmonica = 22,
    Bandoneon = 23,

    // TODO: Fill out the rest of these...

    // Sound Effects
    GuitarFretNoise = 120,
    BreathNoise = 121,
    Seashore = 122,
    BirdTweet = 123,
    TelephoneRing = 124,
    Helicopter = 125,
    Applause = 126,
    Gunshot = 127,
};

enum MIDINotes : BYTE
{
    // Octave -1
    C_MINUS1 = 0,
    CSharp_MINUS1 = 1,
    D_MINUS1 = 2,
    DSharp_MINUS1 = 3,
    E_MINUS1 = 4,
    F_MINUS1 = 5,
    FSharp_MINUS1 = 6,
    G_MINUS1 = 7,
    GSharp_MINUS1 = 8,
    A_MINUS1 = 9,
    ASharp_MINUS1 = 10,
    B_MINUS1 = 11,

    // Octave 0
    C0 = 12,
    CSharp0 = 13,
    D0 = 14,
    DSharp0 = 15,
    E0 = 16,
    F0 = 17,
    FSharp0 = 18,
    G0 = 19,
    GSharp0 = 20,
    A0 = 21,
    ASharp0 = 22,
    B0 = 23,

    // Octave 1
    C1 = 24,
    CSharp1 = 25,
    D1 = 26,
    DSharp1 = 27,
    E1 = 28,
    F1 = 29,
    FSharp1 = 30,
    G1 = 31,
    GSharp1 = 32,
    A1 = 33,
    ASharp1 = 34,
    B1 = 35,

    // Octave 2
    C2 = 36,
    CSharp2 = 37,
    D2 = 38,
    DSharp2 = 39,
    E2 = 40,
    F2 = 41,
    FSharp2 = 42,
    G2 = 43,
    GSharp2 = 44,
    A2 = 45,
    ASharp2 = 46,
    B2 = 47,

    // Octave 3
    C3 = 48,
    CSharp3 = 49,
    D3 = 50,
    DSharp3 = 51,
    E3 = 52,
    F3 = 53,
    FSharp3 = 54,
    G3 = 55,
    GSharp3 = 56,
    A3 = 57,
    ASharp3 = 58,
    B3 = 59,

    // Octave 4
    C4 = 60,
    CSharp4 = 61,
    D4 = 62,
    DSharp4 = 63,
    E4 = 64,
    F4 = 65,
    FSharp4 = 66,
    G4 = 67,
    GSharp4 = 68,
    A4 = 69,
    ASharp4 = 70,
    B4 = 71,

    // Octave 5
    C5 = 72,
    CSharp5 = 73,
    D5 = 74,
    DSharp5 = 75,
    E5 = 76,
    F5 = 77,
    FSharp5 = 78,
    G5 = 79,
    GSharp5 = 80,
    A5 = 81,
    ASharp5 = 82,
    B5 = 83,

    // Octave 6
    C6 = 84,
    CSharp6 = 85,
    D6 = 86,
    DSharp6 = 87,
    E6 = 88,
    F6 = 89,
    FSharp6 = 90,
    G6 = 91,
    GSharp6 = 92,
    A6 = 93,
    ASharp6 = 94,
    B6 = 95,

    // Octave 7
    C7 = 96,
    CSharp7 = 97,
    D7 = 98,
    DSharp7 = 99,
    E7 = 100,
    F7 = 101,
    FSharp7 = 102,
    G7 = 103,
    GSharp7 = 104,
    A7 = 105,
    ASharp7 = 106,
    B7 = 107,

    // Octave 8
    C8 = 108,
    CSharp8 = 109,
    D8 = 110,
    DSharp8 = 111,
    E8 = 112,
    F8 = 113,
    FSharp8 = 114,
    G8 = 115,
    GSharp8 = 116,
    A8 = 117,
    ASharp8 = 118,
    B8 = 119,

    // Octave 9
    C9 = 120,
    CSharp9 = 121,
    D9 = 122,
    DSharp9 = 123,
    E9 = 124,
    F9 = 125,
    FSharp9 = 126,
    G9 = 127,
};
