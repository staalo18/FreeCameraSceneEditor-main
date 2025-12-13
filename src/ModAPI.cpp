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

size_t Messaging::FCSEInterface::AddTranslationPoint(float a_time, bool a_easeIn, bool a_easeOut) const noexcept {
    return FCSE::TimelineManager::GetSingleton().AddTranslationPoint(a_time, a_easeIn, a_easeOut);
}

size_t Messaging::FCSEInterface::AddRotationPoint(float a_time, bool a_easeIn, bool a_easeOut) const noexcept {
    return FCSE::TimelineManager::GetSingleton().AddRotationPoint(a_time, a_easeIn, a_easeOut);
}

void Messaging::FCSEInterface::StartRecording() const noexcept {
    FCSE::TimelineManager::GetSingleton().StartRecording();
}

void Messaging::FCSEInterface::StopRecording() const noexcept {
    FCSE::TimelineManager::GetSingleton().StopRecording();
}

size_t Messaging::FCSEInterface::EditTranslationPoint(size_t a_index, float a_time, float a_posX, float a_posY, float a_posZ, bool a_easeIn, bool a_easeOut) const noexcept {
    return FCSE::TimelineManager::GetSingleton().EditTranslationPoint(a_index, a_time, a_posX, a_posY, a_posZ, a_easeIn, a_easeOut);
}

size_t Messaging::FCSEInterface::EditRotationPoint(size_t a_index, float a_time, float a_pitch, float a_yaw, bool a_easeIn, bool a_easeOut) const noexcept {
    return FCSE::TimelineManager::GetSingleton().EditRotationPoint(a_index, a_time, a_pitch, a_yaw, a_easeIn, a_easeOut);
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

void Messaging::FCSEInterface::StartTraversal(float a_speed, bool a_globalEaseIn, bool a_globalEaseOut, bool a_useDuration, float a_duration) const noexcept {
    FCSE::TimelineManager::GetSingleton().StartTraversal(a_speed, a_globalEaseIn, a_globalEaseOut, a_useDuration, a_duration);
}

void Messaging::FCSEInterface::StopTraversal() const noexcept {
    FCSE::TimelineManager::GetSingleton().StopTraversal();
}

bool Messaging::FCSEInterface::IsTraversing() const noexcept {
    return FCSE::TimelineManager::GetSingleton().IsTraversing();
}

bool Messaging::FCSEInterface::ImportTimeline(const char* a_filePath) const noexcept {
    return FCSE::TimelineManager::GetSingleton().ImportTimeline(a_filePath);
}

bool Messaging::FCSEInterface::ExportTimeline(const char* a_filePath) const noexcept {
    return FCSE::TimelineManager::GetSingleton().ExportTimeline(a_filePath);
}

