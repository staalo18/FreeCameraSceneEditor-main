/**
 * FCSE Event Callback Test Plugin - Example
 * 
 * This is a minimal SKSE plugin that demonstrates how to receive
 * timeline playback events from FreeCameraSceneEditor (FCSE).
 * 
 * To use this:
 * 1. Create a new SKSE plugin project
 * 2. Copy FCSE_API.h into your project
 * 3. Use this code as your plugin.cpp
 * 4. Build and load alongside FCSE
 * 5. Start/stop timeline playback in-game
 * 6. Check your plugin's log for event notifications
 */

#include <SKSE/SKSE.h>
#include "FCSE_API.h"  // Copy from FCSE project

using namespace SKSE;
using namespace SKSE::log;

namespace {
    void MessageHandler(SKSE::MessagingInterface::Message* a_msg) {
        // Filter for messages from FreeCameraSceneEditor
        if (!a_msg->sender || strcmp(a_msg->sender, FCSE_API::FCSEPluginName) != 0) {
            return;
        }

        // Handle FCSE timeline events
        switch (a_msg->type) {
        case static_cast<uint32_t>(FCSE_API::FCSEMessage::kTimelinePlaybackStarted): {
            auto* data = static_cast<FCSE_API::FCSETimelineEventData*>(a_msg->data);
            log::info("FCSE Event Received: Timeline {} started playback", data->timelineID);
            break;
        }
        case static_cast<uint32_t>(FCSE_API::FCSEMessage::kTimelinePlaybackStopped): {
            auto* data = static_cast<FCSE_API::FCSETimelineEventData*>(a_msg->data);
            log::info("FCSE Event Received: Timeline {} stopped playback", data->timelineID);
            break;
        }
        default:
            log::warn("Unknown FCSE message type: {}", a_msg->type);
            break;
        }
    }
}

SKSEPluginLoad(const LoadInterface* skse) {
    InitializeLogging();

    const auto* plugin = PluginDeclaration::GetSingleton();
    log::info("{} v{} loading...", plugin->GetName(), plugin->GetVersion());

    Init(skse);

    // Register listener for SKSE messages
    auto* messaging = GetMessagingInterface();
    if (messaging->RegisterListener(MessageHandler)) {
        log::info("Registered SKSE message listener - will receive FCSE events");
    } else {
        log::error("Failed to register SKSE message listener!");
        return false;
    }

    log::info("{} loaded successfully", plugin->GetName());
    return true;
}
