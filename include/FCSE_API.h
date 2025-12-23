#pragma once
#include <functional>
#include <stdint.h>

/*
* For modders: Copy this file into your own project if you wish to use this API
*/
namespace FCSE_API {
	constexpr const auto FCSEPluginName = "FreeCameraSceneEditor";

	// Available FCSE interface versions
	enum class InterfaceVersion : uint8_t {
		V1
	};

	// FCSE's modder interface
	class IVFCSE1 {
	public:
		/// <summary>
		/// Get the thread ID FCSE is running in.
		/// You may compare this with the result of GetCurrentThreadId() to help determine
		/// if you are using the correct thread.
		/// </summary>
		/// <returns>TID</returns>
		[[nodiscard]] virtual unsigned long GetFCSEThreadId() const noexcept = 0;

		/// <summary>
		/// Add a translation point to the camera timeline at a specified position.
		/// </summary>
		/// <param name="a_interpolationMode">Interpolation mode: 0=None, 1=Linear, 2=CubicHermite (default)</param>
		/// <returns> Index of the added point</returns>
		[[nodiscard]] virtual size_t AddTranslationPoint(float a_time, float a_posX, float a_posY, float a_posZ, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept = 0;

		/// <summary>
		/// Add a translation point to the camera timeline relative to a reference object.
		/// The point will track the reference's position plus the offset.
		/// If a_isOffsetRelative is true, the offset is relative to the reference's heading, else world space.
		/// </summary>
		/// <param name="a_interpolationMode">Interpolation mode: 0=None, 1=Linear, 2=CubicHermite (default)</param>
		/// <returns> Index of the added point</returns>
		[[nodiscard]] virtual size_t AddTranslationPointAtRef(float a_time, RE::TESObjectREFR* a_reference, float a_offsetX = 0.0f, float a_offsetY = 0.0f, float a_offsetZ = 0.0f, bool a_isOffsetRelative = false, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept = 0;

		/// <summary>
		/// Add a translation point to the camera timeline at the current camera position.
		/// </summary>
		/// <param name="a_interpolationMode">Interpolation mode: 0=None, 1=Linear, 2=CubicHermite (default)</param>
		/// <returns> Index of the added point</returns>
		[[nodiscard]] virtual size_t AddTranslationPointAtCamera(float a_time, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept = 0;

		/// <summary>
		/// Add a rotation point to the camera timeline with specified pitch and yaw.
		/// </summary>
		/// <param name="a_interpolationMode">Interpolation mode: 0=None, 1=Linear, 2=CubicHermite (default)</param>
		/// <returns> Index of the added point</returns>
		[[nodiscard]] virtual size_t AddRotationPoint(float a_time, float a_pitch, float a_yaw, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept = 0;

		/// <summary>
		/// Add a rotation point that uses a reference's rotation angles plus offset.
		/// </summary>
		/// <param name="a_interpolationMode">Interpolation mode: 0=None, 1=Linear, 2=CubicHermite (default)</param>
		/// <returns> Index of the added point</returns>
		[[nodiscard]] virtual size_t AddRotationPointAtRef(float a_time, RE::TESObjectREFR* a_reference, float a_offsetPitch, float a_offsetYaw, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept = 0;

		/// <summary>
		/// Add a rotation point to the camera timeline at the current camera rotation.
		/// </summary>
		/// <param name="a_interpolationMode">Interpolation mode: 0=None, 1=Linear, 2=CubicHermite (default)</param>
		/// <returns> Index of the added point</returns>
		[[nodiscard]] virtual size_t AddRotationPointAtCamera(float a_time, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept = 0;

		/// <summary>
		/// Start recording camera movement to the timeline.
		/// </summary>
		virtual void StartRecording() const noexcept = 0;

		/// <summary>
		/// Stop recording camera movement.
		/// </summary>
		virtual void StopRecording() const noexcept = 0;

		/// <summary>
		/// Edit an existing translation point (returns new index after potential re-sorting).
		/// </summary>
		/// <param name="a_interpolationMode">Interpolation mode: 0=None, 1=Linear, 2=CubicHermite (default)</param>
		/// <returns> New index of the point after update</returns>
		[[nodiscard]] virtual size_t EditTranslationPoint(size_t a_index, float a_time, float a_posX, float a_posY, float a_posZ, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept = 0;

		/// <summary>
		/// Edit an existing rotation point (returns new index after potential re-sorting).
		/// </summary>
		/// <param name="a_interpolationMode">Interpolation mode: 0=None, 1=Linear, 2=CubicHermite (default)</param>
		/// <returns> New index of the point after update</returns>
		[[nodiscard]] virtual size_t EditRotationPoint(size_t a_index, float a_time, float a_pitch, float a_yaw, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept = 0;

		/// <summary>
		/// Remove a translation point from the timeline.
		/// </summary>
		virtual void RemoveTranslationPoint(size_t a_index) const noexcept = 0;

		/// <summary>
		/// Remove a rotation point from the timeline.
		/// </summary>
		virtual void RemoveRotationPoint(size_t a_index) const noexcept = 0;

		/// <summary>
		/// Clear the entire timeline. Prints a notification if a_notifyUser is true.
		/// </summary>
		virtual void ClearTimeline(bool a_notifyUser = true) const noexcept = 0;

		/// <summary>
		/// Get the number of translation points in the timeline.
		/// </summary>
		/// <returns> Number of translation points</returns>
		[[nodiscard]] virtual size_t GetTranslationPointCount() const noexcept = 0;

		/// <summary>
		/// Get the number of rotation points in the timeline.
		/// </summary>
		/// <returns> Number of rotation points</returns>
		[[nodiscard]] virtual size_t GetRotationPointCount() const noexcept = 0;

		/// <summary>
		/// Start traversing (playing back) the camera timeline.
		/// If a_useDuration is true: 
        ///     plays timeline over a_duration seconds
		/// If a_useDuration is false:
        ///     plays timeline with a_speed as speed multiplier.
        /// Global easing (a_globalEaseIn/Out) applies to overall playback in both modes.
		/// </summary>
		virtual void StartTraversal(float a_speed = 1.0f, bool a_globalEaseIn = false, bool a_globalEaseOut = false, bool a_useDuration = false, float a_duration = 0.0f) const noexcept = 0;

		/// <summary>
		/// Stop traversing the camera timeline.
		/// </summary>
		virtual void StopTraversal() const noexcept = 0;

		/// <summary>
		/// Check if timeline traversal is currently active.
		/// </summary>
		/// <returns> True if traversing, false otherwise</returns>
		[[nodiscard]] virtual bool IsTraversing() const noexcept = 0;

		/// <summary>
		/// Import camera timeline from a file.
		/// </summary>
		/// <returns> True if successful, false otherwise</returns>
		[[nodiscard]] virtual bool ImportTimeline(const char* a_filePath) const noexcept = 0;

		/// <summary>
		/// Export camera timeline to a file.
		/// </summary>
		/// <returns> True if successful, false otherwise</returns>
		[[nodiscard]] virtual bool ExportTimeline(const char* a_filePath) const noexcept = 0;
	};

	typedef void* (*_RequestPluginAPI)(const InterfaceVersion interfaceVersion);

	/// <summary>
	/// Request the IDRC API interface.
	/// Recommended: Send your request during or after SKSEMessagingInterface::kMessage_PostLoad to make sure the dll has already been loaded
	/// </summary>
	/// <param name="a_interfaceVersion">The interface version to request</param>
	/// <returns>The pointer to the API singleton, or nullptr if request failed</returns>
	[[nodiscard]] inline void* RequestPluginAPI(const InterfaceVersion a_interfaceVersion = InterfaceVersion::V1) {
		auto pluginHandle = GetModuleHandle("IntuitiveDragonRideControl.dll");
		_RequestPluginAPI requestAPIFunction = (_RequestPluginAPI)GetProcAddress(pluginHandle, "RequestPluginAPI");
		if (requestAPIFunction) {
			return requestAPIFunction(a_interfaceVersion);
		}
		return nullptr;
	}
}
