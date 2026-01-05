#include "ModAPI.h"
#include "TimelineManager.h"
#include "CameraPath.h"
#include "CameraTypes.h"

Messaging::FCSEInterface::FCSEInterface() noexcept {
	apiTID = GetCurrentThreadId();
}

Messaging::FCSEInterface::~FCSEInterface() noexcept {}

unsigned long Messaging::FCSEInterface::GetFCSEThreadId() const noexcept {
	return apiTID;
}

int Messaging::FCSEInterface::GetFCSEPluginVersion() const noexcept {
	// Encode version as: major * 10000 + minor * 100 + patch
	return static_cast<int>(Plugin::VERSION[0]) * 10000 + 
	       static_cast<int>(Plugin::VERSION[1]) * 100 + 
	       static_cast<int>(Plugin::VERSION[2]);
}

size_t Messaging::FCSEInterface::RegisterTimeline(SKSE::PluginHandle a_pluginHandle) const noexcept {
	size_t result = FCSE::TimelineManager::GetSingleton().RegisterTimeline(a_pluginHandle);
	log::info("{}: API wrapper returning timeline ID {}", __FUNCTION__, result);
	return result;
}

bool Messaging::FCSEInterface::UnregisterTimeline(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID) const noexcept {
	return FCSE::TimelineManager::GetSingleton().UnregisterTimeline(a_pluginHandle, a_timelineID);
}

int Messaging::FCSEInterface::AddTranslationPoint(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID, float a_time, float a_posX, float a_posY, float a_posZ, bool a_easeIn, bool a_easeOut, int a_interpolationMode) const noexcept {
	return FCSE::TimelineManager::GetSingleton().AddTranslationPoint(a_pluginHandle, a_timelineID, a_time, a_posX, a_posY, a_posZ, a_easeIn, a_easeOut, FCSE::ToInterpolationMode(a_interpolationMode));
}

int Messaging::FCSEInterface::AddTranslationPointAtRef(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID, float a_time, RE::TESObjectREFR* a_reference, float a_offsetX, float a_offsetY, float a_offsetZ, bool a_isOffsetRelative, bool a_easeIn, bool a_easeOut, int a_interpolationMode) const noexcept {
	return FCSE::TimelineManager::GetSingleton().AddTranslationPointAtRef(a_pluginHandle, a_timelineID, a_time, a_reference, a_offsetX, a_offsetY, a_offsetZ, a_isOffsetRelative, a_easeIn, a_easeOut, FCSE::ToInterpolationMode(a_interpolationMode));
}

int Messaging::FCSEInterface::AddTranslationPointAtCamera(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID, float a_time, bool a_easeIn, bool a_easeOut, int a_interpolationMode) const noexcept {
	return FCSE::TimelineManager::GetSingleton().AddTranslationPointAtCamera(a_pluginHandle, a_timelineID, a_time, a_easeIn, a_easeOut, FCSE::ToInterpolationMode(a_interpolationMode));
}

int Messaging::FCSEInterface::AddRotationPoint(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID, float a_time, float a_pitch, float a_yaw, bool a_easeIn, bool a_easeOut, int a_interpolationMode) const noexcept {
	return FCSE::TimelineManager::GetSingleton().AddRotationPoint(a_pluginHandle, a_timelineID, a_time, a_pitch, a_yaw, a_easeIn, a_easeOut, FCSE::ToInterpolationMode(a_interpolationMode));
}

int Messaging::FCSEInterface::AddRotationPointAtRef(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID, float a_time, RE::TESObjectREFR* a_reference, float a_offsetPitch, float a_offsetYaw, bool a_isOffsetRelative, bool a_easeIn, bool a_easeOut, int a_interpolationMode) const noexcept {
	return FCSE::TimelineManager::GetSingleton().AddRotationPointAtRef(a_pluginHandle, a_timelineID, a_time, a_reference, a_offsetPitch, a_offsetYaw, a_isOffsetRelative, a_easeIn, a_easeOut, FCSE::ToInterpolationMode(a_interpolationMode));
}

int Messaging::FCSEInterface::AddRotationPointAtCamera(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID, float a_time, bool a_easeIn, bool a_easeOut, int a_interpolationMode) const noexcept {
    return FCSE::TimelineManager::GetSingleton().AddRotationPointAtCamera(a_pluginHandle, a_timelineID, a_time, a_easeIn, a_easeOut, FCSE::ToInterpolationMode(a_interpolationMode));
}

bool Messaging::FCSEInterface::StartRecording(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID) const noexcept {
    return FCSE::TimelineManager::GetSingleton().StartRecording(a_pluginHandle, a_timelineID);
}

bool Messaging::FCSEInterface::StopRecording(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID) const noexcept {
    return FCSE::TimelineManager::GetSingleton().StopRecording(a_pluginHandle, a_timelineID);
}

bool Messaging::FCSEInterface::RemoveTranslationPoint(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID, size_t a_index) const noexcept {
	return FCSE::TimelineManager::GetSingleton().RemoveTranslationPoint(a_pluginHandle, a_timelineID, a_index);
}

bool Messaging::FCSEInterface::RemoveRotationPoint(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID, size_t a_index) const noexcept {
	return FCSE::TimelineManager::GetSingleton().RemoveRotationPoint(a_pluginHandle, a_timelineID, a_index);
}

bool Messaging::FCSEInterface::ClearTimeline(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID, bool a_notifyUser) const noexcept {
	return FCSE::TimelineManager::GetSingleton().ClearTimeline(a_pluginHandle, a_timelineID, a_notifyUser);
}

int Messaging::FCSEInterface::GetTranslationPointCount(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID) const noexcept {
    return FCSE::TimelineManager::GetSingleton().GetTranslationPointCount(a_pluginHandle, a_timelineID);
}

int Messaging::FCSEInterface::GetRotationPointCount(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID) const noexcept {
	return FCSE::TimelineManager::GetSingleton().GetRotationPointCount(a_pluginHandle, a_timelineID);
}

bool Messaging::FCSEInterface::StartPlayback(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID, float a_speed, bool a_globalEaseIn, bool a_globalEaseOut, bool a_useDuration, float a_duration) const noexcept {
	return FCSE::TimelineManager::GetSingleton().StartPlayback(a_pluginHandle, a_timelineID, a_speed, a_globalEaseIn, a_globalEaseOut, a_useDuration, a_duration);
}

bool Messaging::FCSEInterface::StopPlayback(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID) const noexcept {
	return FCSE::TimelineManager::GetSingleton().StopPlayback(a_pluginHandle, a_timelineID);
}

bool Messaging::FCSEInterface::SwitchPlayback(SKSE::PluginHandle a_pluginHandle, size_t a_fromTimelineID, size_t a_toTimelineID) const noexcept {
	return FCSE::TimelineManager::GetSingleton().SwitchPlayback(a_pluginHandle, a_fromTimelineID, a_toTimelineID);
}

bool Messaging::FCSEInterface::PausePlayback(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID) const noexcept {
	return FCSE::TimelineManager::GetSingleton().PausePlayback(a_pluginHandle, a_timelineID);
}

bool Messaging::FCSEInterface::ResumePlayback(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID) const noexcept {
	return FCSE::TimelineManager::GetSingleton().ResumePlayback(a_pluginHandle, a_timelineID);
}

bool Messaging::FCSEInterface::IsPlaybackRunning(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID) const noexcept {
	return FCSE::TimelineManager::GetSingleton().IsPlaybackRunning(a_pluginHandle, a_timelineID);
}

bool Messaging::FCSEInterface::IsPlaybackPaused(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID) const noexcept {
    return FCSE::TimelineManager::GetSingleton().IsPlaybackPaused(a_pluginHandle, a_timelineID);
}

bool Messaging::FCSEInterface::IsRecording(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID) const noexcept {
    return FCSE::TimelineManager::GetSingleton().IsRecording(a_pluginHandle, a_timelineID);
}

size_t Messaging::FCSEInterface::GetActiveTimelineID() const noexcept {
    return FCSE::TimelineManager::GetSingleton().GetActiveTimelineID();
}

void Messaging::FCSEInterface::AllowUserRotation(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID, bool a_allow) const noexcept {
    FCSE::TimelineManager::GetSingleton().AllowUserRotation(a_pluginHandle, a_timelineID, a_allow);
}

bool Messaging::FCSEInterface::IsUserRotationAllowed(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID) const noexcept {
    return FCSE::TimelineManager::GetSingleton().IsUserRotationAllowed(a_pluginHandle, a_timelineID);
}

bool Messaging::FCSEInterface::SetPlaybackMode(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID, int a_playbackMode) const noexcept {
    return FCSE::TimelineManager::GetSingleton().SetPlaybackMode(a_pluginHandle, a_timelineID, a_playbackMode);
}

bool Messaging::FCSEInterface::AddTimelineFromFile(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID, const char* a_filePath, float a_timeOffset) const noexcept {
    return FCSE::TimelineManager::GetSingleton().AddTimelineFromFile(a_pluginHandle, a_timelineID, a_filePath, a_timeOffset);
}

bool Messaging::FCSEInterface::ExportTimeline(SKSE::PluginHandle a_pluginHandle, size_t a_timelineID, const char* a_filePath) const noexcept {
    return FCSE::TimelineManager::GetSingleton().ExportTimeline(a_pluginHandle, a_timelineID, a_filePath);
}

