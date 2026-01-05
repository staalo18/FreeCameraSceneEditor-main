#include "Hooks.h"
#include "ControlsManager.h"
#include "APIManager.h"
#include "TimelineManager.h"
#include "ModAPI.h"
#include "CameraTypes.h"

namespace FCSE {
    namespace Interface {
        int GetFCSEPluginVersion(RE::StaticFunctionTag*) {
            // Encode version as: major * 10000 + minor * 100 + patch
            return static_cast<int>(Plugin::VERSION[0]) * 10000 + 
                   static_cast<int>(Plugin::VERSION[1]) * 100 + 
                   static_cast<int>(Plugin::VERSION[2]);
        }

        int RegisterTimeline(RE::StaticFunctionTag*, RE::BSFixedString a_modName) {
            if (a_modName.empty()) {
                log::error("{}: Empty mod name provided", __FUNCTION__);
                return -1;
            }

            SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
            if (handle == 0) {
                log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
                return -1;
            }

            return static_cast<int>(FCSE::TimelineManager::GetSingleton().RegisterTimeline(handle));
        }

        bool UnregisterTimeline(RE::StaticFunctionTag*, RE::BSFixedString a_modName, int a_timelineID) {
            if (a_modName.empty() || a_timelineID <= 0) {
                return false;
            }

            SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
            if (handle == 0) {
                log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
                return false;
            }

            return FCSE::TimelineManager::GetSingleton().UnregisterTimeline(static_cast<size_t>(a_timelineID), handle);
        }
        
        int AddTranslationPointAtCamera(RE::StaticFunctionTag*, RE::BSFixedString a_modName, int a_timelineID, float a_time, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
            if (a_modName.empty() || a_timelineID <= 0) {
                return false;
            }

            SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
            if (handle == 0) {
                log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
                return -1;
            }

            return FCSE::TimelineManager::GetSingleton().AddTranslationPointAtCamera(static_cast<size_t>(a_timelineID), handle, a_time, a_easeIn, a_easeOut, ToInterpolationMode(a_interpolationMode));
        }

        int AddTranslationPoint(RE::StaticFunctionTag*, RE::BSFixedString a_modName, int a_timelineID, float a_time, float a_posX, float a_posY, float a_posZ, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
            if (a_modName.empty() || a_timelineID <= 0) {
                return false;
            }

            SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
            if (handle == 0) {
                log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
                return -1;
            }

            return FCSE::TimelineManager::GetSingleton().AddTranslationPoint(static_cast<size_t>(a_timelineID), handle, a_time, a_posX, a_posY, a_posZ, a_easeIn, a_easeOut, ToInterpolationMode(a_interpolationMode));
        }

        int AddTranslationPointAtRef(RE::StaticFunctionTag*, RE::BSFixedString a_modName, int a_timelineID, float a_time, RE::TESObjectREFR* a_reference, float a_offsetX, float a_offsetY, float a_offsetZ, bool a_isOffsetRelative, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
            if (a_modName.empty() || a_timelineID <= 0) {
                return false;
            }

            SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
            if (handle == 0) {
                log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
                return -1;
            }

            return FCSE::TimelineManager::GetSingleton().AddTranslationPointAtRef(static_cast<size_t>(a_timelineID), handle, a_time, a_reference, a_offsetX, a_offsetY, a_offsetZ, a_isOffsetRelative, a_easeIn, a_easeOut, ToInterpolationMode(a_interpolationMode));
        }

        int AddRotationPointAtCamera(RE::StaticFunctionTag*, RE::BSFixedString a_modName, int a_timelineID, float a_time, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
            if (a_modName.empty() || a_timelineID <= 0) {
                return false;
            }

            SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
            if (handle == 0) {
                log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
                return -1;
            }

            return FCSE::TimelineManager::GetSingleton().AddRotationPointAtCamera(static_cast<size_t>(a_timelineID), handle, a_time, a_easeIn, a_easeOut, ToInterpolationMode(a_interpolationMode));
        }

        int AddRotationPoint(RE::StaticFunctionTag*, RE::BSFixedString a_modName, int a_timelineID, float a_time, float a_pitch, float a_yaw, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
            if (a_modName.empty() || a_timelineID <= 0) {
                return false;
            }

            SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
            if (handle == 0) {
                log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
                return -1;
            }

            return FCSE::TimelineManager::GetSingleton().AddRotationPoint(static_cast<size_t>(a_timelineID), handle, a_time, a_pitch, a_yaw, a_easeIn, a_easeOut, ToInterpolationMode(a_interpolationMode));
        }

        int AddRotationPointAtRef(RE::StaticFunctionTag*, RE::BSFixedString a_modName, int a_timelineID, float a_time, RE::TESObjectREFR* a_reference, float a_offsetPitch, float a_offsetYaw, bool a_isOffsetRelative, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
            if (a_modName.empty() || a_timelineID <= 0) {
                return false;
            }

            SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
            if (handle == 0) {
                log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
                return -1;
            }

            return FCSE::TimelineManager::GetSingleton().AddRotationPointAtRef(static_cast<size_t>(a_timelineID), handle, a_time, a_reference, a_offsetPitch, a_offsetYaw, a_isOffsetRelative, a_easeIn, a_easeOut, ToInterpolationMode(a_interpolationMode));
        }

        bool StartRecording(RE::StaticFunctionTag*, RE::BSFixedString a_modName, int a_timelineID) {
            if (a_modName.empty() || a_timelineID <= 0) {
                return false;
            }

            SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
            if (handle == 0) {
                log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
                return false;
            }

            return FCSE::TimelineManager::GetSingleton().StartRecording(static_cast<size_t>(a_timelineID), handle);
        }

        bool StopRecording(RE::StaticFunctionTag*, RE::BSFixedString a_modName, int a_timelineID) {
            if (a_modName.empty() || a_timelineID <= 0) {
                return false;
            }

            SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
            if (handle == 0) {
                log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
                return false;
            }

            return FCSE::TimelineManager::GetSingleton().StopRecording(static_cast<size_t>(a_timelineID), handle);
        }
        
        bool RemoveTranslationPoint(RE::StaticFunctionTag*, RE::BSFixedString a_modName, int a_timelineID, int a_index) {
            if (a_modName.empty() || a_timelineID <= 0) {
                return false;
            }

            SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
            if (handle == 0) {
                log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
                return false;
            }

            return FCSE::TimelineManager::GetSingleton().RemoveTranslationPoint(static_cast<size_t>(a_timelineID), handle, static_cast<size_t>(a_index));
        }

        bool RemoveRotationPoint(RE::StaticFunctionTag*, RE::BSFixedString a_modName, int a_timelineID, int a_index) {
            if (a_modName.empty() || a_timelineID <= 0) {
                return false;
            }

            SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
            if (handle == 0) {
                log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
                return false;
            }

            return FCSE::TimelineManager::GetSingleton().RemoveRotationPoint(static_cast<size_t>(a_timelineID), handle, static_cast<size_t>(a_index));
        }

        bool ClearTimeline(RE::StaticFunctionTag*, RE::BSFixedString a_modName, int a_timelineID, bool a_notifyUser) {
            if (a_modName.empty() || a_timelineID <= 0) {
                return false;
            }

            SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
                if (handle == 0) {
                log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
                return false;
            }

            return FCSE::TimelineManager::GetSingleton().ClearTimeline(static_cast<size_t>(a_timelineID), handle, a_notifyUser);
        }

        int GetTranslationPointCount(RE::StaticFunctionTag*, int a_timelineID) {
            return FCSE::TimelineManager::GetSingleton().GetTranslationPointCount(static_cast<size_t>(a_timelineID));
        }

        int GetRotationPointCount(RE::StaticFunctionTag*, int a_timelineID) {
            return FCSE::TimelineManager::GetSingleton().GetRotationPointCount(static_cast<size_t>(a_timelineID));
        }

        bool StartPlayback(RE::StaticFunctionTag*, RE::BSFixedString a_modName, int a_timelineID, float a_speed, bool a_globalEaseIn, bool a_globalEaseOut, bool a_useDuration, float a_duration) {
            if (a_modName.empty() || a_timelineID <= 0) {
                return false;
            }

            SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
            if (handle == 0) {
                log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
                return false;
            }

            return FCSE::TimelineManager::GetSingleton().StartPlayback(static_cast<size_t>(a_timelineID), a_speed, a_globalEaseIn, a_globalEaseOut, a_useDuration, a_duration);
        }
        
        bool StopPlayback(RE::StaticFunctionTag*, RE::BSFixedString a_modName, int a_timelineID) {
            if (a_modName.empty() || a_timelineID <= 0) {
                return false;
            }

            SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
            if (handle == 0) {
                log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
                return false;
            }

            return FCSE::TimelineManager::GetSingleton().StopPlayback(static_cast<size_t>(a_timelineID));
        }
        
        bool SwitchPlayback(RE::StaticFunctionTag*, RE::BSFixedString a_modName, int a_fromTimelineID, int a_toTimelineID) {
            if (a_modName.empty() || a_toTimelineID <= 0 || a_fromTimelineID < 0) {
                return false;
            }

            SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
            if (handle == 0) {
                log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
                return false;
            }

            return FCSE::TimelineManager::GetSingleton().SwitchPlayback(static_cast<size_t>(a_fromTimelineID), static_cast<size_t>(a_toTimelineID), handle);
        }
        
        bool PausePlayback(RE::StaticFunctionTag*, RE::BSFixedString a_modName, int a_timelineID) {
            if (a_modName.empty() || a_timelineID <= 0) {
                return false;
            }

            SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
            if (handle == 0) {
                log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
                return false;
            }

            return FCSE::TimelineManager::GetSingleton().PausePlayback(static_cast<size_t>(a_timelineID));
        }
        
        bool ResumePlayback(RE::StaticFunctionTag*, RE::BSFixedString a_modName, int a_timelineID) {
            if (a_modName.empty() || a_timelineID <= 0) {
                return false;
            }

            SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
            if (handle == 0) {
                log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
                return false;
            }

            return FCSE::TimelineManager::GetSingleton().ResumePlayback(static_cast<size_t>(a_timelineID));
        }

        bool IsPlaybackPaused(RE::StaticFunctionTag*, int a_timelineID) {
            return FCSE::TimelineManager::GetSingleton().IsPlaybackPaused(static_cast<size_t>(a_timelineID));
        }
        
        bool IsPlaybackRunning(RE::StaticFunctionTag*, std::int32_t a_timelineID) {
            return FCSE::TimelineManager::GetSingleton().IsPlaybackRunning(static_cast<size_t>(a_timelineID));
        }

        bool IsRecording(RE::StaticFunctionTag*, std::int32_t a_timelineID) {
            return FCSE::TimelineManager::GetSingleton().IsRecording(static_cast<size_t>(a_timelineID));
        }

        std::int32_t GetActiveTimelineID(RE::StaticFunctionTag*) {
            return static_cast<std::int32_t>(FCSE::TimelineManager::GetSingleton().GetActiveTimelineID());
        }

        bool AllowUserRotation(RE::StaticFunctionTag*, RE::BSFixedString a_modName, std::int32_t a_timelineID, bool a_allow) {
            if (a_modName.empty() || a_timelineID <= 0) {
                return false;
            }

            SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
            if (handle == 0) {
                log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
                return false;
            }

            return FCSE::TimelineManager::GetSingleton().AllowUserRotation(static_cast<size_t>(a_timelineID), a_allow);
        }

        bool IsUserRotationAllowed(RE::StaticFunctionTag*, std::int32_t a_timelineID) {
            return FCSE::TimelineManager::GetSingleton().IsUserRotationAllowed(static_cast<size_t>(a_timelineID));
        }

        bool SetPlaybackMode(RE::StaticFunctionTag*, RE::BSFixedString a_modName, std::int32_t a_timelineID, std::int32_t a_playbackMode) {
            if (a_modName.empty() || a_timelineID <= 0) {
                return false;
            }

            SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
            if (handle == 0) {
                log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
                return false;
            }

            return FCSE::TimelineManager::GetSingleton().SetPlaybackMode(static_cast<size_t>(a_timelineID), handle, a_playbackMode);
        }

        bool AddTimelineFromFile(RE::StaticFunctionTag*, RE::BSFixedString a_modName, std::int32_t a_timelineID, RE::BSFixedString a_filePath, float a_timeOffset) {
            if (a_modName.empty() || a_timelineID <= 0) {
                return false;
            }
            
            SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
            if (handle == 0) {
                log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
                return false;
            }
            
            return FCSE::TimelineManager::GetSingleton().AddTimelineFromFile(static_cast<size_t>(a_timelineID), handle, a_filePath.c_str(), a_timeOffset);
        }

        bool ExportTimeline(RE::StaticFunctionTag*, RE::BSFixedString a_modName, std::int32_t a_timelineID, RE::BSFixedString a_filePath) {
            if (a_modName.empty() || a_timelineID <= 0) {
                return false;
            }

            SKSE::PluginHandle handle = FCSE::ModNameToHandle(a_modName.c_str());
            if (handle == 0) {
                log::error("{}: Invalid mod name '{}' - mod not loaded or doesn't exist", __FUNCTION__, a_modName.c_str());
                return false;
            }            

            return FCSE::TimelineManager::GetSingleton().ExportTimeline(static_cast<size_t>(a_timelineID), a_filePath.c_str());
        }
        
        void RegisterForTimelineEvents(RE::StaticFunctionTag*, RE::TESForm* a_form) {
            if (!a_form) {
                log::error("{}: Null form provided", __FUNCTION__);
                return;
            }
            
            FCSE::TimelineManager::GetSingleton().RegisterForTimelineEvents(a_form);
        }
        
        void UnregisterForTimelineEvents(RE::StaticFunctionTag*, RE::TESForm* a_form) {
            if (!a_form) {
                log::error("{}: Null form provided", __FUNCTION__);
                return;
            }
            
            FCSE::TimelineManager::GetSingleton().UnregisterForTimelineEvents(a_form);
        }

        bool FCSEFunctions(RE::BSScript::Internal::VirtualMachine * a_vm){
            a_vm->RegisterFunction("FCSE_GetPluginVersion", "FCSE_SKSEFunctions", GetFCSEPluginVersion);
            a_vm->RegisterFunction("FCSE_RegisterTimeline", "FCSE_SKSEFunctions", RegisterTimeline);
            a_vm->RegisterFunction("FCSE_UnregisterTimeline", "FCSE_SKSEFunctions", UnregisterTimeline);
            a_vm->RegisterFunction("FCSE_AddTranslationPointAtCamera", "FCSE_SKSEFunctions", AddTranslationPointAtCamera);
            a_vm->RegisterFunction("FCSE_AddTranslationPoint", "FCSE_SKSEFunctions", AddTranslationPoint);
            a_vm->RegisterFunction("FCSE_AddTranslationPointAtRef", "FCSE_SKSEFunctions", AddTranslationPointAtRef);
            a_vm->RegisterFunction("FCSE_AddRotationPointAtCamera", "FCSE_SKSEFunctions", AddRotationPointAtCamera);
            a_vm->RegisterFunction("FCSE_AddRotationPoint", "FCSE_SKSEFunctions", AddRotationPoint);
            a_vm->RegisterFunction("FCSE_AddRotationPointAtRef", "FCSE_SKSEFunctions", AddRotationPointAtRef);
            a_vm->RegisterFunction("FCSE_StartRecording", "FCSE_SKSEFunctions", StartRecording);
            a_vm->RegisterFunction("FCSE_StopRecording", "FCSE_SKSEFunctions", StopRecording);
            a_vm->RegisterFunction("FCSE_RemoveTranslationPoint", "FCSE_SKSEFunctions", RemoveTranslationPoint);
            a_vm->RegisterFunction("FCSE_RemoveRotationPoint", "FCSE_SKSEFunctions", RemoveRotationPoint);
            a_vm->RegisterFunction("FCSE_ClearTimeline", "FCSE_SKSEFunctions", ClearTimeline);
            a_vm->RegisterFunction("FCSE_GetTranslationPointCount", "FCSE_SKSEFunctions", GetTranslationPointCount);
            a_vm->RegisterFunction("FCSE_GetRotationPointCount", "FCSE_SKSEFunctions", GetRotationPointCount);
            a_vm->RegisterFunction("FCSE_StartPlayback", "FCSE_SKSEFunctions", StartPlayback);
            a_vm->RegisterFunction("FCSE_StopPlayback", "FCSE_SKSEFunctions", StopPlayback);
            a_vm->RegisterFunction("FCSE_SwitchPlayback", "FCSE_SKSEFunctions", SwitchPlayback);
            a_vm->RegisterFunction("FCSE_PausePlayback", "FCSE_SKSEFunctions", PausePlayback);
            a_vm->RegisterFunction("FCSE_ResumePlayback", "FCSE_SKSEFunctions", ResumePlayback);
            a_vm->RegisterFunction("FCSE_IsPlaybackPaused", "FCSE_SKSEFunctions", IsPlaybackPaused);
            a_vm->RegisterFunction("FCSE_IsPlaybackRunning", "FCSE_SKSEFunctions", IsPlaybackRunning);
            a_vm->RegisterFunction("FCSE_IsRecording", "FCSE_SKSEFunctions", IsRecording);
            a_vm->RegisterFunction("FCSE_GetActiveTimelineID", "FCSE_SKSEFunctions", GetActiveTimelineID);
            a_vm->RegisterFunction("FCSE_AllowUserRotation", "FCSE_SKSEFunctions", AllowUserRotation);
            a_vm->RegisterFunction("FCSE_IsUserRotationAllowed", "FCSE_SKSEFunctions", IsUserRotationAllowed);
            a_vm->RegisterFunction("FCSE_SetPlaybackMode", "FCSE_SKSEFunctions", SetPlaybackMode);
            a_vm->RegisterFunction("FCSE_AddTimelineFromFile", "FCSE_SKSEFunctions", AddTimelineFromFile);
            a_vm->RegisterFunction("FCSE_ExportTimeline", "FCSE_SKSEFunctions", ExportTimeline);
            a_vm->RegisterFunction("FCSE_RegisterForTimelineEvents", "FCSE_SKSEFunctions", RegisterForTimelineEvents);
            a_vm->RegisterFunction("FCSE_UnregisterForTimelineEvents", "FCSE_SKSEFunctions", UnregisterForTimelineEvents);
            return true;
        }
    } // namespace Interface
} // namespace FCSE

/******************************************************************************************/
void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
	// Try requesting APIs at multiple steps to try to work around the SKSE messaging bug
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		APIs::RequestAPIs();
		break;
	case SKSE::MessagingInterface::kPostLoad:
		APIs::RequestAPIs();
		break;
	case SKSE::MessagingInterface::kPostPostLoad:
		APIs::RequestAPIs();
		break;
	case SKSE::MessagingInterface::kPreLoadGame:
		break;
	case SKSE::MessagingInterface::kPostLoadGame:
	case SKSE::MessagingInterface::kNewGame:
		APIs::RequestAPIs();
        if (auto input = RE::BSInputDeviceManager::GetSingleton()) {
            input->AddEventSink(&FCSE::ControlsManager::GetSingleton());
        } else {
                log::warn("{}: BSInputDeviceManager not available", __FUNCTION__);
        }   
		break;
	}
}
/******************************************************************************************/
SKSEPluginInfo(
    .Version = Plugin::VERSION,
    .Name = Plugin::NAME,
    .Author = "Staalo",
    .RuntimeCompatibility = SKSE::PluginDeclaration::RuntimeCompatibility(SKSE::VersionIndependence::AddressLibrary),
    .MinimumSKSEVersion = { 2, 2, 3 } // or 0 if you want to support all
)

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
    long logLevel = _ts_SKSEFunctions::GetValueFromINI(nullptr, 0, "LogLevel:Log", "SKSE/Plugins/FreeCameraSceneEditor.ini", 3L);
    bool isLogLevelValid = true;
    if (logLevel < 0 || logLevel > 6) {
        logLevel = 2L; // info
        isLogLevelValid = false;
    }

	_ts_SKSEFunctions::InitializeLogging(static_cast<spdlog::level::level_enum>(logLevel));

    Init(skse);
    auto messaging = SKSE::GetMessagingInterface();
	if (!messaging->RegisterListener("SKSE", MessageHandler)) {
		return false;
	}

    if (!isLogLevelValid) {
        log::warn("{}: LogLevel in INI file is invalid. Defaulting to info level.", __FUNCTION__);
    }
    log::info("{}: LogLevel: {}, FCSE Plugin version: {}", __FUNCTION__, logLevel, FCSE::Interface::GetFCSEPluginVersion(nullptr));

    if (!SKSE::GetPapyrusInterface()->Register(FCSE::Interface::FCSEFunctions)) {
        log::warn("{}: Failed to register Papyrus functions.", __FUNCTION__);
        return false;
    } else {
        log::info("{}: Registered Papyrus functions", __FUNCTION__);
    }

    SKSE::AllocTrampoline(64);
        
    log::info("{}: Calling Install Hooks", __FUNCTION__);

    Hooks::Install();

    return true;
}

extern "C" DLLEXPORT void* SKSEAPI RequestPluginAPI(const FCSE_API::InterfaceVersion a_interfaceVersion)
{
	auto api = Messaging::FCSEInterface::GetSingleton();

	log::info("{} called, InterfaceVersion {}", __FUNCTION__, static_cast<uint8_t>(a_interfaceVersion));

	switch (a_interfaceVersion) {
	case FCSE_API::InterfaceVersion::V1:
		log::info("{} returned the API singleton", __FUNCTION__);
		return static_cast<void*>(api);
	}

	log::info("{} requested the wrong interface version", __FUNCTION__);
	return nullptr;
}
