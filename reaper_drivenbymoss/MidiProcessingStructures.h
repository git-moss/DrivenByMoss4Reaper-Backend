// Copyright (c) 2018-2025 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_MIDI_PROCESSING_STRUCTURES_H_
#define _DBM_MIDI_PROCESSING_STRUCTURES_H_

#include <array>
#include <numeric>
#include <unordered_map>

#include "CodeAnalysis.h"


// Immutable per-note-input filter set
using FilterSet = std::vector<std::vector<unsigned char>>;

constexpr size_t MAX_NOTE_INPUTS = 6;


/**
 * Copy of key-/velocity tables for fast lookup.
 */
struct FastNoteLookup
{
    // Lookup table: [noteInputIndex][noteNumber]
    std::array<std::array<int, 128>, MAX_NOTE_INPUTS> keyLookup;
    std::array<std::array<int, 128>, MAX_NOTE_INPUTS> velocityLookup;

    // Filter matching table: [noteInputIndex][status][data1]
    // true = match
    std::array<std::array<std::array<bool, 128>, 256>, MAX_NOTE_INPUTS> filterMatch{};
};


/**
 * Structure for key and velocity translation tables.
 */
struct NoteInputData
{
    // still needed for rebuilding
    FilterSet filters;
    std::array<int, 128> keyTable{};
    std::array<int, 128> velocityTable{};

    NoteInputData() noexcept
    {
        // Block all by default
        DISABLE_WARNING_USE_GSL_AT
        DISABLE_WARNING_ACCESS_ARRAYS_WITH_CONST
        for (int i = 0; i < 128; ++i)
        {
            keyTable[i] = -1;
            velocityTable[i] = -1;
        }
    }
};


/**
 * Structure for note input lookup tables.
 */
struct DeviceNoteData
{
    std::array<NoteInputData, MAX_NOTE_INPUTS> noteInputs;

    // Lookup table for RT thread
    FastNoteLookup lookup;


    void BuildLookup()
    {
        // Do not use gsl:at for performance reasons!
        DISABLE_WARNING_USE_GSL_AT
        DISABLE_WARNING_ACCESS_ARRAYS_WITH_CONST

        for (int noteIdx = 0; noteIdx < MAX_NOTE_INPUTS; ++noteIdx)
        {
            const auto& noteData = noteInputs[noteIdx];

            // Copy keyTable & velocityTable for fast runtime access
            for (int i = 0; i < 128; ++i)
            {
                lookup.keyLookup[noteIdx][i] = noteData.keyTable[i];
                lookup.velocityLookup[noteIdx][i] = noteData.velocityTable[i];
            }

            // Build filter match table
            for (const auto& filter : noteData.filters)
            {
                if (filter.size() == 1)
                    lookup.filterMatch[noteIdx][filter[0]].fill(true);
                else if (filter.size() == 2)
                    lookup.filterMatch[noteIdx][filter[0]][filter[1]] = true;
            }
        }
    }
};


#endif /* _DBM_MIDI_PROCESSING_STRUCTURES_H_ */