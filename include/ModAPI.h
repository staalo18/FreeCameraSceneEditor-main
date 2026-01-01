#pragma once
#include "FCSE_API.h"

namespace Messaging
{
	using InterfaceVersion1 = ::FCSE_API::IVFCSE1;

	class FCSEInterface : public InterfaceVersion1
	{
	private:
		FCSEInterface() noexcept;
		virtual ~FCSEInterface() noexcept;

	public:
		static FCSEInterface* GetSingleton() noexcept
		{
			static FCSEInterface singleton;
			return std::addressof(singleton);
		}

		// InterfaceVersion1
        virtual unsigned long GetFCSEThreadId(void) const noexcept override;
		virtual int GetFCSEPluginVersion() const noexcept override;
        virtual int AddTranslationPoint(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, float a_posX, float a_posY, float a_posZ, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept override;
        virtual int AddTranslationPointAtRef(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, RE::TESObjectREFR* a_reference, float a_offsetX = 0.0f, float a_offsetY = 0.0f, float a_offsetZ = 0.0f, bool a_isOffsetRelative = false, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept override;
		virtual int AddTranslationPointAtCamera(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept override;
		virtual int AddRotationPoint(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, float a_pitch, float a_yaw, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept override;
		virtual int AddRotationPointAtRef(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, RE::TESObjectREFR* a_reference, float a_offsetPitch, float a_offsetYaw, bool a_isOffsetRelative = false, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept override;
		virtual int AddRotationPointAtCamera(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept override;
		virtual bool StartRecording(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle) const noexcept override;
		virtual bool StopRecording(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle) const noexcept override;
		virtual bool RemoveTranslationPoint(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, size_t a_index) const noexcept override;
		virtual bool RemoveRotationPoint(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, size_t a_index) const noexcept override;
		virtual bool ClearTimeline(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, bool a_notifyUser = true) const noexcept override;
		virtual int GetTranslationPointCount(size_t a_timelineID) const noexcept override;
		virtual int GetRotationPointCount(size_t a_timelineID) const noexcept override;
		virtual bool StartPlayback(size_t a_timelineID, float a_speed = 1.0f, bool a_globalEaseIn = false, bool a_globalEaseOut = false, bool a_useDuration = false, float a_duration = 0.0f) const noexcept override;
		virtual bool StopPlayback(size_t a_timelineID) const noexcept override;
		virtual bool PausePlayback(size_t a_timelineID) const noexcept override;
		virtual bool ResumePlayback(size_t a_timelineID) const noexcept override;
		virtual bool IsPlaybackRunning(size_t a_timelineID) const noexcept override;
		virtual bool IsRecording(size_t a_timelineID) const noexcept override;
		virtual bool IsPlaybackPaused(size_t a_timelineID) const noexcept override;
		virtual size_t GetActiveTimelineID() const noexcept override;
		virtual void AllowUserRotation(size_t a_timelineID, bool a_allow) const noexcept override;
		virtual bool IsUserRotationAllowed(size_t a_timelineID) const noexcept override;
		virtual bool AddTimelineFromFile(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, const char* a_filePath, float a_timeOffset = 0.0f) const noexcept override;
		virtual bool ExportTimeline(size_t a_timelineID, const char* a_filePath) const noexcept override;

	private:
		unsigned long apiTID = 0;
	};
}
