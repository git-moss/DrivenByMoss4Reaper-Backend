// Copyright (c) 2018-2025 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_LOCAL_MIDI_EVENT_DISPATCHER_H_
#define _DBM_LOCAL_MIDI_EVENT_DISPATCHER_H_

#include <unordered_map>
#include <queue>
#include <mutex>
#include <vector>

#include "reaper_plugin.h"

constexpr size_t MAX_EVENTS_PER_DEVICE = 1024; // adjust as needed

/**
 * Helper class to insert MIDI events into the Reaper device queues from Java.
 */
class LocalMidiEventDispatcher
{
    std::mutex producerMutex;
    std::unordered_map<int, std::queue<MIDI_event_t>> producerQueues;
    std::unordered_map<int, std::vector<MIDI_event_t>> rtLocalBuffers;

public:

    LocalMidiEventDispatcher()
    {
        for (auto& bufferPair : rtLocalBuffers)
            bufferPair.second.reserve(MAX_EVENTS_PER_DEVICE);
    }

    // Called by producers from any thread
    void Push(int deviceID, const MIDI_event_t& evt)
    {
        std::lock_guard<std::mutex> lock(producerMutex);
        producerQueues[deviceID].push(evt);
    }

    // Called by the RT audio thread
    void ProcessDeviceQueue(int deviceID, MIDI_eventlist* eventList)
    {
        if (eventList == nullptr)
            return;

        auto& localBuffer = rtLocalBuffers[deviceID];
        localBuffer.clear(); // reuse preallocated space

        // Move events from producer queues to RT local buffers
        {
            std::lock_guard<std::mutex> lock(producerMutex);
            auto& queue = producerQueues[deviceID];
            while (!queue.empty())
            {
                localBuffer.push_back(queue.front());
                queue.pop();
            }
        }

        // Step 2: Now RT thread has exclusive access to rtLocalBuffers, no locks needed
        for (auto& event : rtLocalBuffers[deviceID])
            eventList->AddItem(&event);
    }
};

#endif /* _DBM_LOCAL_MIDI_EVENT_DISPATCHER_H_ */
