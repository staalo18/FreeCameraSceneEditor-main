#include "Hooks.h"
#include "ControlsManager.h"
#include "APIManager.h"
#include "TimelineManager.h"
#include "ModAPI.h"

namespace FCSE {
    namespace Interface {
        int GetFCSEPluginVersion(RE::StaticFunctionTag*) {
            return 1;
        }
        
        int AddTranslationPoint(RE::StaticFunctionTag*, float a_time, bool a_easeIn, bool a_easeOut) {
            return static_cast<int>(FCSE::TimelineManager::GetSingleton().AddTranslationPoint(a_time, a_easeIn, a_easeOut));
        }

        int AddRotationPoint(RE::StaticFunctionTag*, float a_time, bool a_easeIn, bool a_easeOut) {
            return static_cast<int>(FCSE::TimelineManager::GetSingleton().AddRotationPoint(a_time, a_easeIn, a_easeOut));
        }

        void StartRecording(RE::StaticFunctionTag*) {
            FCSE::TimelineManager::GetSingleton().StartRecording();
        }

        void StopRecording(RE::StaticFunctionTag*) {
            FCSE::TimelineManager::GetSingleton().StopRecording();
        }

        int EditTranslationPoint(RE::StaticFunctionTag*, int a_index, float a_time, float a_posX, float a_posY, float a_posZ, bool a_easeIn, bool a_easeOut) {
            return static_cast<int>(FCSE::TimelineManager::GetSingleton().EditTranslationPoint(static_cast<size_t>(a_index), a_time, a_posX, a_posY, a_posZ, a_easeIn, a_easeOut));
        }

        int EditRotationPoint(RE::StaticFunctionTag*, int a_index, float a_time, float a_pitch, float a_yaw, bool a_easeIn, bool a_easeOut) {
            return static_cast<int>(FCSE::TimelineManager::GetSingleton().EditRotationPoint(static_cast<size_t>(a_index), a_time, a_pitch, a_yaw, a_easeIn, a_easeOut));
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

        void StartTraversal(RE::StaticFunctionTag*, float a_speed, bool a_globalEaseIn, bool a_globalEaseOut, bool a_useDuration, float a_duration) {
            FCSE::TimelineManager::GetSingleton().StartTraversal(a_speed, a_globalEaseIn, a_globalEaseOut, a_useDuration, a_duration);
        }

        void StopTraversal(RE::StaticFunctionTag*) {
            FCSE::TimelineManager::GetSingleton().StopTraversal();
        }

        bool IsTraversing(RE::StaticFunctionTag*) {
            return FCSE::TimelineManager::GetSingleton().IsTraversing();
        }

        bool ImportTimeline(RE::StaticFunctionTag*, RE::BSFixedString a_filePath) {
            return FCSE::TimelineManager::GetSingleton().ImportTimeline(a_filePath.c_str());
        }

        bool ExportTimeline(RE::StaticFunctionTag*, RE::BSFixedString a_filePath) {
            return FCSE::TimelineManager::GetSingleton().ExportTimeline(a_filePath.c_str());
        }

        bool FCSEFunctions(RE::BSScript::Internal::VirtualMachine * a_vm){
            a_vm->RegisterFunction("GetFCSEPluginVersion", "FCSE_SKSEFunctions", GetFCSEPluginVersion);
            a_vm->RegisterFunction("AddTranslationPoint", "FCSE_SKSEFunctions", AddTranslationPoint);
            a_vm->RegisterFunction("AddRotationPoint", "FCSE_SKSEFunctions", AddRotationPoint);
            a_vm->RegisterFunction("StartRecording", "FCSE_SKSEFunctions", StartRecording);
            a_vm->RegisterFunction("StopRecording", "FCSE_SKSEFunctions", StopRecording);
            a_vm->RegisterFunction("EditTranslationPoint", "FCSE_SKSEFunctions", EditTranslationPoint);
            a_vm->RegisterFunction("EditRotationPoint", "FCSE_SKSEFunctions", EditRotationPoint);
            a_vm->RegisterFunction("RemoveTranslationPoint", "FCSE_SKSEFunctions", RemoveTranslationPoint);
            a_vm->RegisterFunction("RemoveRotationPoint", "FCSE_SKSEFunctions", RemoveRotationPoint);
            a_vm->RegisterFunction("ClearTimeline", "FCSE_SKSEFunctions", ClearTimeline);
            a_vm->RegisterFunction("GetTranslationPointCount", "FCSE_SKSEFunctions", GetTranslationPointCount);
            a_vm->RegisterFunction("GetRotationPointCount", "FCSE_SKSEFunctions", GetRotationPointCount);
            a_vm->RegisterFunction("StartTraversal", "FCSE_SKSEFunctions", StartTraversal);
            a_vm->RegisterFunction("StopTraversal", "FCSE_SKSEFunctions", StopTraversal);
            a_vm->RegisterFunction("IsTraversing", "FCSE_SKSEFunctions", IsTraversing);
            a_vm->RegisterFunction("ImportTimeline", "FCSE_SKSEFunctions", ImportTimeline);
            a_vm->RegisterFunction("ExportTimeline", "FCSE_SKSEFunctions", ExportTimeline);
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
