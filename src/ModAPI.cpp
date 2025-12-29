#include "ModAPI.h"
#include "TimelineManager.h"
#include "CameraPath.h"

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

size_t Messaging::FCSEInterface::AddTranslationPoint(float a_time, float a_posX, float a_posY, float a_posZ, bool a_easeIn, bool a_easeOut, int a_interpolationMode) const noexcept {
    return FCSE::TimelineManager::GetSingleton().AddTranslationPoint(a_time, a_posX, a_posY, a_posZ, a_easeIn, a_easeOut, a_interpolationMode);
}

size_t Messaging::FCSEInterface::AddTranslationPointAtRef(float a_time, RE::TESObjectREFR* a_reference, float a_offsetX, float a_offsetY, float a_offsetZ, bool a_isOffsetRelative, bool a_easeIn, bool a_easeOut, int a_interpolationMode) const noexcept {
    return FCSE::TimelineManager::GetSingleton().AddTranslationPointAtRef(a_time, a_reference, a_offsetX, a_offsetY, a_offsetZ, a_isOffsetRelative, a_easeIn, a_easeOut, a_interpolationMode);
}

size_t Messaging::FCSEInterface::AddTranslationPointAtCamera(float a_time, bool a_easeIn, bool a_easeOut, int a_interpolationMode) const noexcept {
    return FCSE::TimelineManager::GetSingleton().AddTranslationPointAtCamera(a_time, a_easeIn, a_easeOut, a_interpolationMode);
}

size_t Messaging::FCSEInterface::AddRotationPoint(float a_time, float a_pitch, float a_yaw, bool a_easeIn, bool a_easeOut, int a_interpolationMode) const noexcept {
    return FCSE::TimelineManager::GetSingleton().AddRotationPoint(a_time, a_pitch, a_yaw, a_easeIn, a_easeOut, a_interpolationMode);
}

size_t Messaging::FCSEInterface::AddRotationPointAtRef(float a_time, RE::TESObjectREFR* a_reference, float a_offsetPitch, float a_offsetYaw, bool a_isOffsetRelative, bool a_easeIn, bool a_easeOut, int a_interpolationMode) const noexcept {
    return FCSE::TimelineManager::GetSingleton().AddRotationPointAtRef(a_time, a_reference, a_offsetPitch, a_offsetYaw, a_isOffsetRelative, a_easeIn, a_easeOut, a_interpolationMode);
}

size_t Messaging::FCSEInterface::AddRotationPointAtCamera(float a_time, bool a_easeIn, bool a_easeOut, int a_interpolationMode) const noexcept {
    return FCSE::TimelineManager::GetSingleton().AddRotationPointAtCamera(a_time, a_easeIn, a_easeOut, a_interpolationMode);
}

void Messaging::FCSEInterface::StartRecording() const noexcept {
    FCSE::TimelineManager::GetSingleton().StartRecording();
}

void Messaging::FCSEInterface::StopRecording() const noexcept {
    FCSE::TimelineManager::GetSingleton().StopRecording();
}

void Messaging::FCSEInterface::RemoveTranslationPoint(size_t a_index) const noexcept {
    FCSE::TimelineManager::GetSingleton().RemoveTranslationPoint(a_index);
}

void Messaging::FCSEInterface::RemoveRotationPoint(size_t a_index) const noexcept {
    FCSE::TimelineManager::GetSingleton().RemoveRotationPoint(a_index);
}

void Messaging::FCSEInterface::ClearTimeline(bool a_notifyUser) const noexcept {
    FCSE::TimelineManager::GetSingleton().ClearTimeline(a_notifyUser);
}

size_t Messaging::FCSEInterface::GetTranslationPointCount() const noexcept {
    return FCSE::TimelineManager::GetSingleton().GetTranslationPointCount();
}

size_t Messaging::FCSEInterface::GetRotationPointCount() const noexcept {
    return FCSE::TimelineManager::GetSingleton().GetRotationPointCount();
}

void Messaging::FCSEInterface::StartPlayback(float a_speed, bool a_globalEaseIn, bool a_globalEaseOut, bool a_useDuration, float a_duration) const noexcept {
    FCSE::TimelineManager::GetSingleton().StartPlayback(a_speed, a_globalEaseIn, a_globalEaseOut, a_useDuration, a_duration);
}

void Messaging::FCSEInterface::StopPlayback() const noexcept {
    FCSE::TimelineManager::GetSingleton().StopPlayback();
}

void Messaging::FCSEInterface::PausePlayback() const noexcept {
    FCSE::TimelineManager::GetSingleton().PausePlayback();
}

void Messaging::FCSEInterface::ResumePlayback() const noexcept {
    FCSE::TimelineManager::GetSingleton().ResumePlayback();
}

bool Messaging::FCSEInterface::IsPlaybackPaused() const noexcept {
    return FCSE::TimelineManager::GetSingleton().IsPlaybackPaused();
}

void Messaging::FCSEInterface::AllowUserRotation(bool a_allow) const noexcept {
    FCSE::TimelineManager::GetSingleton().AllowUserRotation(a_allow);
}

bool Messaging::FCSEInterface::IsUserRotationAllowed() const noexcept {
    return FCSE::TimelineManager::GetSingleton().IsUserRotationAllowed();
}

bool Messaging::FCSEInterface::IsPlaybackRunning() const noexcept {
    return FCSE::TimelineManager::GetSingleton().IsPlaybackRunning();
}

bool Messaging::FCSEInterface::AddTimelineFromFile(const char* a_filePath, float a_timeOffset) const noexcept {
    return FCSE::TimelineManager::GetSingleton().AddTimelineFromFile(a_filePath, a_timeOffset);
}

bool Messaging::FCSEInterface::ExportTimeline(const char* a_filePath) const noexcept {
    return FCSE::TimelineManager::GetSingleton().ExportTimeline(a_filePath);
}

