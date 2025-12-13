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
		virtual size_t AddTranslationPoint(float a_time, bool a_easeIn, bool a_easeOut) const noexcept override;
		virtual size_t AddRotationPoint(float a_time, bool a_easeIn, bool a_easeOut) const noexcept override;
		virtual void StartRecording() const noexcept override;
		virtual void StopRecording() const noexcept override;
		virtual size_t EditTranslationPoint(size_t a_index, float a_time, float a_posX, float a_posY, float a_posZ, bool a_easeIn, bool a_easeOut) const noexcept override;
		virtual size_t EditRotationPoint(size_t a_index, float a_time, float a_pitch, float a_yaw, bool a_easeIn, bool a_easeOut) const noexcept override;
		virtual void RemoveTranslationPoint(size_t a_index) const noexcept override;
		virtual void RemoveRotationPoint(size_t a_index) const noexcept override;
		virtual void ClearTimeline(bool a_notifyUser) const noexcept override;
		virtual size_t GetTranslationPointCount() const noexcept override;
		virtual size_t GetRotationPointCount() const noexcept override;
		virtual void StartTraversal() const noexcept override;
		virtual void StopTraversal() const noexcept override;
		virtual bool IsTraversing() const noexcept override;
		virtual bool ImportTimeline(const char* a_filePath) const noexcept override;
		virtual bool ExportTimeline(const char* a_filePath) const noexcept override;

	private:
		unsigned long apiTID = 0;
	};
}
