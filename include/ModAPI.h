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
		virtual size_t AddTranslationPoint(float a_time, float a_posX, float a_posY, float a_posZ, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept override;
		virtual size_t AddTranslationPointAtRef(float a_time, RE::TESObjectREFR* a_reference, float a_offsetX, float a_offsetY, float a_offsetZ, bool a_isOffsetRelative = false, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept override;
		virtual size_t AddTranslationPointAtCamera(float a_time, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept override;
		virtual size_t AddRotationPoint(float a_time, float a_pitch, float a_yaw, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept override;
		virtual size_t AddRotationPointAtRef(float a_time, RE::TESObjectREFR* a_reference, float a_offsetPitch, float a_offsetYaw, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept override;
		virtual size_t AddRotationPointAtCamera(float a_time, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept override;
		virtual void StartRecording() const noexcept override;
		virtual void StopRecording() const noexcept override;
		virtual size_t EditTranslationPoint(size_t a_index, float a_time, float a_posX, float a_posY, float a_posZ, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept override;
		virtual size_t EditRotationPoint(size_t a_index, float a_time, float a_pitch, float a_yaw, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept override;
		virtual void RemoveTranslationPoint(size_t a_index) const noexcept override;
		virtual void RemoveRotationPoint(size_t a_index) const noexcept override;
		virtual void ClearTimeline(bool a_notifyUser = true) const noexcept override;
		virtual size_t GetTranslationPointCount() const noexcept override;
		virtual size_t GetRotationPointCount() const noexcept override;
        void StartPlayback(float a_speed = 1.0f, bool a_globalEaseIn = false, bool a_globalEaseOut = false, bool a_useDuration = false, float a_duration = 0.0f) const noexcept override;
		virtual void StopPlayback() const noexcept override;
		virtual void PausePlayback() const noexcept override;
		virtual void ResumePlayback() const noexcept override;
		virtual bool IsPlaybackPaused() const noexcept override;
		virtual void AllowUserRotation(bool a_allow) const noexcept override;
		virtual bool IsUserRotationAllowed() const noexcept override;
		virtual bool IsPlaybackRunning() const noexcept override;
		virtual bool AddTimelineFromFile(const char* a_filePath, float a_timeOffset = 0.0f) const noexcept override;
		virtual bool ExportTimeline(const char* a_filePath) const noexcept override;

	private:
		unsigned long apiTID = 0;
	};
}
