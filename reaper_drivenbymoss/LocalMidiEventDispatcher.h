// Copyright (c) 2018-2025 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_LOCAL_MIDI_EVENT_DISPATCHER_H_
#define _DBM_LOCAL_MIDI_EVENT_DISPATCHER_H_

#include <unordered_map>
#include <queue>
#include <mutex>
#include <vector>

#include "reaper_plugin.h"


/**
 * Helper class to insert MIDI events into the Reaper device queues from Java.
 */
class LocalMidiEventDispatcher
{
    std::unordered_map<int, std::queue<MIDI_event_t>> producerQueues;
    std::mutex producerMutex;
    std::unordered_map<int, std::vector<MIDI_event_t>> rtLocalBuffers;

public:
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

        // Move events from producer queues to RT local buffers
        {
            std::lock_guard<std::mutex> lock(producerMutex);
            while (!producerQueues[deviceID].empty())
            {
                rtLocalBuffers[deviceID].push_back(producerQueues[deviceID].front());
                producerQueues[deviceID].pop();
            }
        }

        // Step 2: Now RT thread has exclusive access to rtLocalBuffers, no locks needed
        for (auto& event : rtLocalBuffers[deviceID])
            eventList->AddItem(&event);
        rtLocalBuffers[deviceID].clear();
    }
};

#endif /* _DBM_LOCAL_MIDI_EVENT_DISPATCHER_H_ */
