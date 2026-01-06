// Copyright (c) 2018-2026 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_MIDI_MESSAGES_H_
#define _DBM_MIDI_MESSAGES_H_

#include <cstdint>
#include <cstring>
#include <type_traits>

/* ──────────────────────────────────────────────────────────────── */
/* 1.  Short (1‑ to 3‑byte) MIDI messages                           */
/* ──────────────────────────────────────────────────────────────── */
struct Midi3 {
    uint32_t deviceId;
    uint8_t  status;   // first byte – 0x8n..0xEn or 0xF8..0xFF
    uint8_t  data1;    // 0 if not used
    uint8_t  data2;    // 0 if not used
};
static_assert(std::is_trivially_copyable<Midi3>::value, "");

/* ──────────────────────────────────────────────────────────────── */
/* 2.  SysEx up to 1 024 bytes                                      */
/* ──────────────────────────────────────────────────────────────── */
constexpr jsize kSyx1k_Max = 1'024;

struct MidiSyx1k {
    uint32_t deviceId;
    uint32_t size;                  // 1 … 1 024
    uint8_t  data[kSyx1k_Max];
};
static_assert(std::is_trivially_copyable<MidiSyx1k>::value, "");

/* ──────────────────────────────────────────────────────────────── */
/* 3.  Rare SysEx up to 65 536 bytes                                */
/* ──────────────────────────────────────────────────────────────── */
constexpr jsize kSyx64k_Max = 65'536;

struct MidiSyx64k {
    uint32_t deviceId;
    uint32_t size;                  // 1 … 65 536
    uint8_t  data[kSyx64k_Max];
};
static_assert(std::is_trivially_copyable<MidiSyx64k>::value, "");

#endif /* _DBM_MIDI_MESSAGES_H_ */
