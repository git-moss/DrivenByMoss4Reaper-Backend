// Copyright (c) 2018-2025 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_MIDI_PROCESSING_STRUCTURES_H_
#define _DBM_MIDI_PROCESSING_STRUCTURES_H_

#include <array>
#include <numeric>
#include <unordered_map>

#include "CodeAnalysis.h"


constexpr size_t MAX_NOTE_INPUTS = 16;


// Immutable per-note-input filter set
using FilterSet = std::vector<std::vector<unsigned char>>;


/**
 * Structure for key and velocity translation tables.
 */
struct NoteInputData
{
    // Needs to be stored for rebuilding the fast lookup table! in DeviceNoteData
    FilterSet filters;
    std::array<int, 128> keyTable{};
    std::array<int, 128> velocityTable{};

    NoteInputData() noexcept
    {
        DISABLE_WARNING_USE_GSL_AT
        DISABLE_WARNING_ACCESS_ARRAYS_WITH_CONST
        // Default is identity
        for (int i = 0; i < 128; ++i)
        {
            keyTable[i] = i;
            velocityTable[i] = i;
        }
    }
};


/**
 * Structure for note input lookup tables.
 */
struct DeviceNoteData
{
    std::array<NoteInputData, MAX_NOTE_INPUTS> noteInputs;

    // Lookup tables for RT thread: [noteInputIndex][noteNumber]
    std::array<std::array<int, 128>, MAX_NOTE_INPUTS> keyLookup;
    std::array<std::array<int, 128>, MAX_NOTE_INPUTS> velocityLookup;

    // Filter matching table for RT thread: [noteInputIndex][status][data1], true = match
    std::array<std::array<std::array<bool, 128>, 256>, MAX_NOTE_INPUTS> filterMatch{};


    DeviceNoteData() noexcept
    {
        this->BuildKeyLookup();
        this->BuildVelocityLookup();
        this->BuildFilterLookup();
    }


    // Copy keyTable for fast runtime access
    void BuildKeyLookup() noexcept
    {
        // Do not use gsl:at for performance reasons!
        DISABLE_WARNING_USE_GSL_AT
        DISABLE_WARNING_ACCESS_ARRAYS_WITH_CONST

        for (int noteIdx = 0; noteIdx < MAX_NOTE_INPUTS; ++noteIdx)
        {
            const auto& noteData = noteInputs[noteIdx];
            for (int i = 0; i < 128; ++i)
                this->keyLookup[noteIdx][i] = noteData.keyTable[i];
        }
    }


    // Copy velocityTable for fast runtime access
    void BuildVelocityLookup() noexcept
    {
        // Do not use gsl:at for performance reasons!
        DISABLE_WARNING_USE_GSL_AT
        DISABLE_WARNING_ACCESS_ARRAYS_WITH_CONST

        for (int noteIdx = 0; noteIdx < MAX_NOTE_INPUTS; ++noteIdx)
        {
            const auto& noteData = noteInputs[noteIdx];
            for (int i = 0; i < 128; ++i)
                this->velocityLookup[noteIdx][i] = noteData.velocityTable[i];
        }
    }


    // Build filter match table
    void BuildFilterLookup() noexcept
    {
        // Do not use gsl:at for performance reasons!
        DISABLE_WARNING_USE_GSL_AT
        DISABLE_WARNING_ACCESS_ARRAYS_WITH_CONST

        for (int noteIdx = 0; noteIdx < MAX_NOTE_INPUTS; ++noteIdx)
        {
            const auto& noteData = noteInputs[noteIdx];
            for (const auto& filter : noteData.filters)
            {
                if (filter.size() == 1)
                {
                    auto& array = this->filterMatch[noteIdx][filter[0]];
                    for ( int i = 0; i < array.size(); i++)
                        array[i] = true;
                }
                else if (filter.size() == 2)
                    this->filterMatch[noteIdx][filter[0]][filter[1]] = true;
            }
        }
    }
};


#endif /* _DBM_MIDI_PROCESSING_STRUCTURES_H_ */