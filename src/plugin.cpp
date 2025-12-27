#include "Hooks.h"
#include "ControlsManager.h"
#include "APIManager.h"
#include "TimelineManager.h"
#include "ModAPI.h"

namespace FCSE {
    namespace Interface {
        int GetFCSEPluginVersion(RE::StaticFunctionTag*) {
            // Encode version as: major * 10000 + minor * 100 + patch
            return static_cast<int>(Plugin::VERSION[0]) * 10000 + 
                   static_cast<int>(Plugin::VERSION[1]) * 100 + 
                   static_cast<int>(Plugin::VERSION[2]);
        }
        
        int AddTranslationPointAtCamera(RE::StaticFunctionTag*, float a_time, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
            return static_cast<int>(FCSE::TimelineManager::GetSingleton().AddTranslationPointAtCamera(a_time, a_easeIn, a_easeOut, a_interpolationMode));
        }

        int AddTranslationPoint(RE::StaticFunctionTag*, float a_time, float a_posX, float a_posY, float a_posZ, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
            return static_cast<int>(FCSE::TimelineManager::GetSingleton().AddTranslationPoint(a_time, a_posX, a_posY, a_posZ, a_easeIn, a_easeOut, a_interpolationMode));
        }

        int AddTranslationPointAtRef(RE::StaticFunctionTag*, float a_time, RE::TESObjectREFR* a_reference, float a_offsetX, float a_offsetY, float a_offsetZ, bool a_isOffsetRelative, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
            return static_cast<int>(FCSE::TimelineManager::GetSingleton().AddTranslationPointAtRef(a_time, a_reference, a_offsetX, a_offsetY, a_offsetZ, a_isOffsetRelative, a_easeIn, a_easeOut, a_interpolationMode));
        }

        int AddRotationPointAtCamera(RE::StaticFunctionTag*, float a_time, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
            return static_cast<int>(FCSE::TimelineManager::GetSingleton().AddRotationPointAtCamera(a_time, a_easeIn, a_easeOut, a_interpolationMode));
        }

        int AddRotationPoint(RE::StaticFunctionTag*, float a_time, float a_pitch, float a_yaw, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
            return static_cast<int>(FCSE::TimelineManager::GetSingleton().AddRotationPoint(a_time, a_pitch, a_yaw, a_easeIn, a_easeOut, a_interpolationMode));
        }

        int AddRotationPointAtRef(RE::StaticFunctionTag*, float a_time, RE::TESObjectREFR* a_reference, float a_offsetPitch, float a_offsetYaw, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
            return static_cast<int>(FCSE::TimelineManager::GetSingleton().AddRotationPointAtRef(a_time, a_reference, a_offsetPitch, a_offsetYaw, a_easeIn, a_easeOut, a_interpolationMode));
        }

        void StartRecording(RE::StaticFunctionTag*) {
            FCSE::TimelineManager::GetSingleton().StartRecording();
        }

        void StopRecording(RE::StaticFunctionTag*) {
            FCSE::TimelineManager::GetSingleton().StopRecording();
        }

        int EditTranslationPoint(RE::StaticFunctionTag*, int a_index, float a_time, float a_posX, float a_posY, float a_posZ, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
            return static_cast<int>(FCSE::TimelineManager::GetSingleton().EditTranslationPoint(static_cast<size_t>(a_index), a_time, a_posX, a_posY, a_posZ, a_easeIn, a_easeOut, a_interpolationMode));
        }

        int EditRotationPoint(RE::StaticFunctionTag*, int a_index, float a_time, float a_pitch, float a_yaw, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
            return static_cast<int>(FCSE::TimelineManager::GetSingleton().EditRotationPoint(static_cast<size_t>(a_index), a_time, a_pitch, a_yaw, a_easeIn, a_easeOut, a_interpolationMode));
        }

        void RemoveTranslationPoint(RE::StaticFunctionTag*, int a_index) {
            FCSE::TimelineManager::GetSingleton().RemoveTranslationPoint(static_cast<size_t>(a_index));
        }

        void RemoveRotationPoint(RE::StaticFunctionTag*, int a_index) {
            FCSE::TimelineManager::GetSingleton().RemoveRotationPoint(static_cast<size_t>(a_index));
        }

        void ClearTimeline(RE::StaticFunctionTag*, bool a_notifyUser) {
            FCSE::TimelineManager::GetSingleton().ClearTimeline(a_notifyUser);
        }

        int GetTranslationPointCount(RE::StaticFunctionTag*) {
            return static_cast<int>(FCSE::TimelineManager::GetSingleton().GetTranslationPointCount());
        }

        int GetRotationPointCount(RE::StaticFunctionTag*) {
            return static_cast<int>(FCSE::TimelineManager::GetSingleton().GetRotationPointCount());
        }

        void StartPlayback(RE::StaticFunctionTag*, float a_speed, bool a_globalEaseIn, bool a_globalEaseOut, bool a_useDuration, float a_duration) {
            FCSE::TimelineManager::GetSingleton().StartPlayback(a_speed, a_globalEaseIn, a_globalEaseOut, a_useDuration, a_duration);
        }

        void StopPlayback(RE::StaticFunctionTag*) {
            FCSE::TimelineManager::GetSingleton().StopPlayback();
        }

        void PausePlayback(RE::StaticFunctionTag*) {
            FCSE::TimelineManager::GetSingleton().PausePlayback();
        }

        void ResumePlayback(RE::StaticFunctionTag*) {
            FCSE::TimelineManager::GetSingleton().ResumePlayback();
        }

        bool IsPlaybackPaused(RE::StaticFunctionTag*) {
            return FCSE::TimelineManager::GetSingleton().IsPlaybackPaused();
        }

        bool IsPlaybackRunning(RE::StaticFunctionTag*) {
            return FCSE::TimelineManager::GetSingleton().IsPlaybackRunning();
        }

        void AllowUserRotation(RE::StaticFunctionTag*, bool a_allow) {
            FCSE::TimelineManager::GetSingleton().AllowUserRotation(a_allow);
        }

        bool IsUserRotationAllowed(RE::StaticFunctionTag*) {
            return FCSE::TimelineManager::GetSingleton().IsUserRotationAllowed();
        }

        bool AddTimelineFromFile(RE::StaticFunctionTag*, RE::BSFixedString a_filePath, float a_timeOffset) {
            return FCSE::TimelineManager::GetSingleton().AddTimelineFromFile(a_filePath.c_str(), a_timeOffset);
        }

        bool ExportTimeline(RE::StaticFunctionTag*, RE::BSFixedString a_filePath) {
            return FCSE::TimelineManager::GetSingleton().ExportTimeline(a_filePath.c_str());
        }

        bool FCSEFunctions(RE::BSScript::Internal::VirtualMachine * a_vm){
            a_vm->RegisterFunction("FCSE_GetPluginVersion", "FCSE_SKSEFunctions", GetFCSEPluginVersion);
            a_vm->RegisterFunction("FCSE_AddTranslationPointAtCamera", "FCSE_SKSEFunctions", AddTranslationPointAtCamera);
            a_vm->RegisterFunction("FCSE_AddTranslationPoint", "FCSE_SKSEFunctions", AddTranslationPoint);
            a_vm->RegisterFunction("FCSE_AddTranslationPointAtRef", "FCSE_SKSEFunctions", AddTranslationPointAtRef);
            a_vm->RegisterFunction("FCSE_AddRotationPointAtCamera", "FCSE_SKSEFunctions", AddRotationPointAtCamera);
            a_vm->RegisterFunction("FCSE_AddRotationPoint", "FCSE_SKSEFunctions", AddRotationPoint);
            a_vm->RegisterFunction("FCSE_AddRotationPointAtRef", "FCSE_SKSEFunctions", AddRotationPointAtRef);
            a_vm->RegisterFunction("FCSE_StartRecording", "FCSE_SKSEFunctions", StartRecording);
            a_vm->RegisterFunction("FCSE_StopRecording", "FCSE_SKSEFunctions", StopRecording);
            a_vm->RegisterFunction("FCSE_EditTranslationPoint", "FCSE_SKSEFunctions", EditTranslationPoint);
            a_vm->RegisterFunction("FCSE_EditRotationPoint", "FCSE_SKSEFunctions", EditRotationPoint);
            a_vm->RegisterFunction("FCSE_RemoveTranslationPoint", "FCSE_SKSEFunctions", RemoveTranslationPoint);
            a_vm->RegisterFunction("FCSE_RemoveRotationPoint", "FCSE_SKSEFunctions", RemoveRotationPoint);
            a_vm->RegisterFunction("FCSE_ClearTimeline", "FCSE_SKSEFunctions", ClearTimeline);
            a_vm->RegisterFunction("FCSE_GetTranslationPointCount", "FCSE_SKSEFunctions", GetTranslationPointCount);
            a_vm->RegisterFunction("FCSE_GetRotationPointCount", "FCSE_SKSEFunctions", GetRotationPointCount);
            a_vm->RegisterFunction("FCSE_StartPlayback", "FCSE_SKSEFunctions", StartPlayback);
            a_vm->RegisterFunction("FCSE_StopPlayback", "FCSE_SKSEFunctions", StopPlayback);
            a_vm->RegisterFunction("FCSE_PausePlayback", "FCSE_SKSEFunctions", PausePlayback);
            a_vm->RegisterFunction("FCSE_ResumePlayback", "FCSE_SKSEFunctions", ResumePlayback);
            a_vm->RegisterFunction("FCSE_IsPlaybackPaused", "FCSE_SKSEFunctions", IsPlaybackPaused);
            a_vm->RegisterFunction("FCSE_IsPlaybackRunning", "FCSE_SKSEFunctions", IsPlaybackRunning);
            a_vm->RegisterFunction("FCSE_AllowUserRotation", "FCSE_SKSEFunctions", AllowUserRotation);
            a_vm->RegisterFunction("FCSE_IsUserRotationAllowed", "FCSE_SKSEFunctions", IsUserRotationAllowed);
            a_vm->RegisterFunction("FCSE_AddTimelineFromFile", "FCSE_SKSEFunctions", AddTimelineFromFile);
            a_vm->RegisterFunction("FCSE_ExportTimeline", "FCSE_SKSEFunctions", ExportTimeline);
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
