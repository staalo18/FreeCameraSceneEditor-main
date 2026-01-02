#pragma once
#include <functional>
#include <stdint.h>

/*
* For modders: Copy this file into your own project if you wish to use this API
*/
namespace FCSE_API {
	constexpr const auto FCSEPluginName = "FreeCameraSceneEditor";

	// SKSE Messaging Interface - Timeline Event Types
	// Consumers can register a listener with SKSE::GetMessagingInterface()->RegisterListener()
	// and receive these messages from FCSE
	enum class FCSEMessage : uint32_t {
		// Dispatched when timeline playback starts
		// Data: FCSETimelineEventData*
		kTimelinePlaybackStarted = 0,
		
		// Dispatched when timeline playback stops (including natural completion or manual stop)
		// Data: FCSETimelineEventData*
		kTimelinePlaybackStopped = 1
	};

	// Event data structure for timeline events
	// Cast the 'data' parameter in your message handler to this type
	struct FCSETimelineEventData {
		size_t timelineID;  // ID of the timeline that triggered the event
	};

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
		/// Get the FCSE plugin version as an integer.
		/// Encoded as: major * 10000 + minor * 100 + patch
		/// Example: version 1.2.3 returns 10203
		/// </summary>
		/// <returns>Version encoded as integer</returns>
		[[nodiscard]] virtual int GetFCSEPluginVersion() const noexcept = 0;

		/// <summary>
		/// Add a translation point at a specified position.
		/// </summary>
		/// <param name="a_timelineID">Timeline ID to add the point to</param>
		/// <param name="a_pluginHandle">Plugin handle of the calling plugin (use SKSE::GetPluginHandle())</param>
		/// <param name="a_time">Time in seconds when this point occurs</param>
		/// <param name="a_posX">X position coordinate</param>
		/// <param name="a_posY">Y position coordinate</param>
		/// <param name="a_posZ">Z position coordinate</param>
		/// <param name="a_easeIn">Apply ease-in at the start of interpolation (default: false)</param>
		/// <param name="a_easeOut">Apply ease-out at the end of interpolation (default: false)</param>
		/// <param name="a_interpolationMode">Interpolation mode: 0=None, 1=Linear, 2=CubicHermite (default)</param>
		/// <returns>Index of the added point, or -1 on failure</returns>
		[[nodiscard]] virtual int AddTranslationPoint(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, float a_posX, float a_posY, float a_posZ, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept = 0;

		/// <summary>
		/// Add a translation point to the camera timeline relative to a reference object.
		/// The point will track the reference's position plus the offset.
		/// If a_isOffsetRelative is true, the offset is relative to the reference's heading, else world space.
		/// </summary>
		/// <param name="a_timelineID">Timeline ID to add the point to</param>
		/// <param name="a_pluginHandle">Plugin handle of the calling plugin (use SKSE::GetPluginHandle())</param>
		/// <param name="a_time">Time in seconds when this point occurs</param>
		/// <param name="a_reference">The object reference to track</param>
		/// <param name="a_offsetX">X offset from reference position (default: 0.0)</param>
		/// <param name="a_offsetY">Y offset from reference position (default: 0.0)</param>
		/// <param name="a_offsetZ">Z offset from reference position (default: 0.0)</param>
		/// <param name="a_isOffsetRelative">If true, offset is relative to reference's heading (local space), otherwise world space (default: false)</param>
		/// <param name="a_easeIn">Apply ease-in at the start of interpolation (default: false)</param>
		/// <param name="a_easeOut">Apply ease-out at the end of interpolation (default: false)</param>
		/// <param name="a_interpolationMode">Interpolation mode: 0=None, 1=Linear, 2=CubicHermite (default)</param>
		/// <returns>Index of the added point, or -1 on failure</returns>
		[[nodiscard]] virtual int AddTranslationPointAtRef(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, RE::TESObjectREFR* a_reference, float a_offsetX = 0.0f, float a_offsetY = 0.0f, float a_offsetZ = 0.0f, bool a_isOffsetRelative = false, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept = 0;

		/// <summary>
		/// Add a translation point that captures camera position at the start of playback.
		/// This point can be used to start playback smoothly from the last camera position, and return to it later.
		/// </summary>
		/// <param name="a_timelineID">Timeline ID to add the point to</param>
		/// <param name="a_pluginHandle">Plugin handle of the calling plugin (use SKSE::GetPluginHandle())</param>
		/// <param name="a_time">Time in seconds when this point occurs</param>
		/// <param name="a_easeIn">Apply ease-in at the start of interpolation (default: false)</param>
		/// <param name="a_easeOut">Apply ease-out at the end of interpolation (default: false)</param>
		/// <param name="a_interpolationMode">Interpolation mode: 0=None, 1=Linear, 2=CubicHermite (default)</param>
		/// <returns> Index of the added point, or -1 on failure</returns>
		[[nodiscard]] virtual int AddTranslationPointAtCamera(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept = 0;

        /// <summary>
        /// Add a rotation point to the camera timeline with specified pitch and yaw.
        /// </summary>
        /// <param name="a_timelineID">Timeline ID to add the point to</param>
        /// <param name="a_pluginHandle">Plugin handle of the calling plugin (use SKSE::GetPluginHandle())</param>
        /// <param name="a_time">Time in seconds when this point occurs</param>
        /// <param name="a_pitch">Pitch rotation in radians (relative to world coordinates)</param>
        /// <param name="a_yaw">Yaw rotation in radians (relative to world coordinates)</param>
        /// <param name="a_easeIn">Apply ease-in at the start of interpolation (default: false)</param>
        /// <param name="a_easeOut">Apply ease-out at the end of interpolation (default: false)</param>
        /// <param name="a_interpolationMode">Interpolation mode: 0=None, 1=Linear, 2=CubicHermite (default)</param>
        /// <returns>Index of the added point, or -1 on failure</returns>
        [[nodiscard]] virtual int AddRotationPoint(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, float a_pitch, float a_yaw, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept = 0;
		
		/// <summary>
		/// Add a rotation point that sets the rotation relative to camera-to-reference direction, or alternatively the ref's heading
		/// </summary>
        /// <param name="a_timelineID">Timeline ID to add the point to</param>
        /// <param name="a_pluginHandle">Plugin handle of the calling plugin (use SKSE::GetPluginHandle())</param>
		/// <param name="a_time">Time in seconds when this point occurs</param>
		/// <param name="a_reference">The object reference to track</param>
		/// <param name="a_offsetPitch">Pitch offset from camera-to-reference direction (a_isOffsetRelative == false) / the ref's heading (a_isOffsetRelative == true)</param>
		/// <param name="a_offsetYaw">a_isOffsetRelative == false - yaw offset from camera-to-reference direction. A value of 0 means looking directly at the reference.
		///            a_isOffsetRelative == true - yaw offset from reference's heading. A value of 0 means looking into the direction the ref is heading.</param>
		/// <param name="a_isOffsetRelative">If true, offset is relative to reference's heading instead of camera-to-reference direction.</param>
		/// <param name="a_easeIn">Ease in at the start of interpolation</param>
		/// <param name="a_easeOut">Ease out at the end of interpolation</param>
		/// <param name="a_interpolationMode">0=None, 1=Linear, 2=CubicHermite (default)</param>
		/// <returns> Index of the added point on success, -1 on failure</returns>
		[[nodiscard]] virtual int AddRotationPointAtRef(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, RE::TESObjectREFR* a_reference, float a_offsetPitch, float a_offsetYaw, bool a_isOffsetRelative = false, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept = 0;

        /// <summary>
        /// Add a rotation point that captures camera rotation at the start of playback.
        /// This point can be used to start playback smoothly from the last camera rotation, and return to it later.
        /// </summary>
        /// <param name="a_timelineID">Timeline ID to add the point to</param>
        /// <param name="a_pluginHandle">Plugin handle of the calling plugin (use SKSE::GetPluginHandle())</param>
        /// <param name="a_time">Time in seconds when this point occurs</param>
        /// <param name="a_easeIn">Apply ease-in at the start of interpolation (default: false)</param>
        /// <param name="a_easeOut">Apply ease-out at the end of interpolation (default: false)</param>
        /// <param name="a_interpolationMode">Interpolation mode: 0=None, 1=Linear, 2=CubicHermite (default)</param>
		/// <returns>Index of the added point, or -1 on failure</returns>
		[[nodiscard]] virtual int AddRotationPointAtCamera(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, bool a_easeIn = false, bool a_easeOut = false, int a_interpolationMode = 2) const noexcept = 0;

		/// <summary>
		/// Remove a translation point from a timeline
		/// </summary>
		/// <param name="a_timelineID">Timeline ID to remove the point from</param>
		/// <param name="a_pluginHandle">Plugin handle of the calling plugin (use SKSE::GetPluginHandle())</param>
		/// <param name="a_index">Index of the point to remove</param>
		/// <returns>true if the point was removed, false on failure</returns>
		[[nodiscard]] virtual bool RemoveTranslationPoint(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, size_t a_index) const noexcept = 0;

        /// <summary>
        /// Start recording camera movement to a timeline.
        /// </summary>
        /// <param name="a_timelineID">Timeline ID to record to</param>
        /// <param name="a_pluginHandle">Plugin handle of the calling plugin (use SKSE::GetPluginHandle())</param>
        /// <returns>True on success, false on failure</returns>
        virtual bool StartRecording(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle) const noexcept = 0;

		/// <summary>
		/// Stop recording camera movements on a timeline.
		/// Adds a final point with ease-out at the current camera position.
		/// </summary>
		/// <param name="a_timelineID">Timeline ID to stop recording on</param>
		/// <param name="a_pluginHandle">Plugin handle of the calling plugin (use SKSE::GetPluginHandle())</param>
		/// <returns>true on success, false on failure</returns>
		virtual bool StopRecording(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle) const noexcept = 0;

		/// <summary>
		/// Remove a rotation point from a timeline
		/// </summary>
		/// <param name="a_timelineID">Timeline ID to remove the point from</param>
		/// <param name="a_pluginHandle">Plugin handle of the calling plugin (use SKSE::GetPluginHandle())</param>
		/// <param name="a_index">Index of the point to remove</param>
		/// <returns>true if the point was removed, false on failure</returns>
		virtual bool RemoveRotationPoint(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, size_t a_index) const noexcept = 0;

		/// <summary>
		/// Clear the entire timeline. Prints a notification if a_notifyUser is true.
		/// </summary>
		/// <param name="a_timelineID">Timeline ID to clear</param>
		/// <param name="a_pluginHandle">Plugin handle of the calling plugin (use SKSE::GetPluginHandle())</param>
		/// <param name="a_notifyUser">Whether to show a notification to the user (default: true)</param>
		/// <returns>true if cleared successfully, false on failure</returns>
		virtual bool ClearTimeline(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, bool a_notifyUser = true) const noexcept = 0;

        /// <summary>
        /// Get the number of translation points in the timeline.
        /// </summary>
        /// <param name="a_timelineID">Timeline ID to query</param>
        /// <returns>Number of translation points, or -1 if timeline not found</returns>
        [[nodiscard]] virtual int GetTranslationPointCount(size_t a_timelineID) const noexcept = 0;

		/// <summary>
		/// Get the number of rotation points in the timeline.
		/// </summary>
		/// <param name="a_timelineID">Timeline ID to query</param>
		/// <returns>Number of rotation points, or -1 if timeline not found</returns>
		[[nodiscard]] virtual int GetRotationPointCount(size_t a_timelineID) const noexcept = 0;

		/// <summary>
		/// Start playing the camera timeline.
		/// If a_useDuration is true: 
        ///     plays complete timeline with total time a_duration seconds
		/// If a_useDuration is false:
        ///     plays timeline with a_speed as speed multiplier.
        /// Global easing (a_globalEaseIn/Out) applies to overall playback in both modes.
		/// </summary>
		/// <param name="a_timelineID">Timeline ID to clear</param>
		/// <param name="a_pluginHandle">Plugin handle of the calling plugin (use SKSE::GetPluginHandle())</param>
		/// <param name="a_speed">Playback speed multiplier, only used if a_useDuration is false (default: 1.0)</param>
		/// <param name="a_globalEaseIn">Apply ease-in at the start of entire playback (default: false)</param>
		/// <param name="a_globalEaseOut">Apply ease-out at the end of entire playback (default: false)</param>
		/// <param name="a_useDuration">If true, plays complete timeline with total time a_duration seconds; if false, uses a_speed multiplier (default: false)</param>
		/// <param name="a_duration">Total duration in seconds for entire timeline, only used if a_useDuration is true (default: 0.0)</param>
		virtual bool StartPlayback(size_t a_timelineID, float a_speed = 1.0f, bool a_globalEaseIn = false, bool a_globalEaseOut = false, bool a_useDuration = false, float a_duration = 0.0f) const noexcept = 0;

		/// <summary>
		/// Stop playback of a camera timeline.
		/// </summary>
		/// <param name="a_timelineID">Timeline ID to stop</param>
		/// <returns>true on success, false on failure</returns>
		[[nodiscard]] virtual bool StopPlayback(size_t a_timelineID) const noexcept = 0;

		/// <summary>
		/// Pause playback of a camera path timeline.
		/// </summary>
		/// <param name="a_timelineID">Timeline ID to pause</param>
		/// <returns>true on success, false on failure</returns>
		[[nodiscard]] virtual bool PausePlayback(size_t a_timelineID) const noexcept = 0;

		/// <summary>
		/// Resume playback of a camera path timeline.
		/// </summary>
		/// <param name="a_timelineID">Timeline ID to resume</param>
		/// <returns>true on success, false on failure</returns>
		[[nodiscard]] virtual bool ResumePlayback(size_t a_timelineID) const noexcept = 0;

		/// <summary>
		/// Check if a timeline is currently playing back.
		/// </summary>
		/// <param name="a_timelineID">Timeline ID to check</param>
		/// <returns>true if playing, false otherwise</returns>
		[[nodiscard]] virtual bool IsPlaybackRunning(size_t a_timelineID) const noexcept = 0;

		/// <summary>
		/// Check if a timeline is currently recording.
		/// </summary>
		/// <param name="a_timelineID">Timeline ID to check</param>
		/// <returns>true if recording, false otherwise</returns>
		[[nodiscard]] virtual bool IsRecording(size_t a_timelineID) const noexcept = 0;

		/// <summary>
		/// Check if timeline playback is currently paused.
		/// </summary>
		/// <param name="a_timelineID">Timeline ID to check</param>
		/// <returns>True if paused, false otherwise</returns>
		[[nodiscard]] virtual bool IsPlaybackPaused(size_t a_timelineID) const noexcept = 0;

		/// <summary>
		/// Get the ID of the currently active timeline (recording or playing).
		/// </summary>
		/// <returns>Timeline ID if active (>0), or 0 if no timeline is active</returns>
		[[nodiscard]] virtual size_t GetActiveTimelineID() const noexcept = 0;

		/// <summary>
		/// Enable or disable user rotation control during playback for a specific timeline.
		/// </summary>
		/// <param name="a_timelineID">Timeline ID to configure</param>
		/// <param name="a_allow">True to allow user to control rotation, false to disable</param>
		virtual void AllowUserRotation(size_t a_timelineID, bool a_allow) const noexcept = 0;

		/// <summary>
		/// Check if user rotation is currently allowed for a specific timeline.
		/// </summary>
		/// <param name="a_timelineID">Timeline ID to check</param>
		/// <returns>True if user can control rotation, false otherwise</returns>
		[[nodiscard]] virtual bool IsUserRotationAllowed(size_t a_timelineID) const noexcept = 0;

		/// <summary>
		/// Adds camera timeline imported from a_filePath at time a_timeOffset to the specified timeline.
		/// </summary>
		/// <param name="a_timelineID">ID of the timeline to add points to</param>
		/// <param name="a_pluginHandle">Plugin handle for ownership validation</param>
		/// <param name="a_filePath">Relative path from Data folder (e.g., "SKSE/Plugins/MyTimeline.dat")</param>
		/// <param name="a_timeOffset">Time offset in seconds to add to all imported point times (default: 0.0)</param>
		/// <returns> True if successful, false otherwise</returns>
		[[nodiscard]] virtual bool AddTimelineFromFile(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, const char* a_filePath, float a_timeOffset = 0.0f) const noexcept = 0;

		/// <summary>
		/// Export camera timeline to a file.
		/// </summary>
		/// <param name="a_timelineID">ID of the timeline to export</param>
		/// <param name="a_filePath">Relative path from Data folder (e.g., "SKSE/Plugins/MyTimeline.dat")</param>
		/// <returns> True if successful, false otherwise</returns>
		[[nodiscard]] virtual bool ExportTimeline(size_t a_timelineID, const char* a_filePath) const noexcept = 0;
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
