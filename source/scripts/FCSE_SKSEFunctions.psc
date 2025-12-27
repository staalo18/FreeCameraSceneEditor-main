Scriptname FCSE_SKSEFunctions

; Get plugin version
; Encoded as: major * 10000 + minor * 100 + patch
; Example: version 1.2.3 returns 10203
int Function FCSE_GetPluginVersion() global native

; Add a translation point at a specified position
; time: time in seconds when this point occurs
; posX, posY, posZ: position coordinates
; easeIn: ease in at the start of interpolation
; easeOut: ease out at the end of interpolation
; interpolationMode: 0=None, 1=Linear, 2=CubicHermite (default)
; Returns: index of the added point
int Function FCSE_AddTranslationPoint(float time, float posX, float posY, float posZ, bool easeIn = false, bool easeOut = false, int interpolationMode = 2) global native

; Add a translation point relative to a reference object
; time: time in seconds when this point occurs
; reference: the object reference to track
; offsetX, offsetY, offsetZ: offset from reference position
; isOffsetRelative: if true, offset is relative to reference's heading (local space), otherwise world space
; easeIn: ease in at the start of interpolation
; easeOut: ease out at the end of interpolation
; interpolationMode: 0=None, 1=Linear, 2=CubicHermite (default)
; Returns: index of the added point
int Function FCSE_AddTranslationPointAtRef(float time, ObjectReference reference, float offsetX, float offsetY, float offsetZ, bool isOffsetRelative = false, bool easeIn = false, bool easeOut = false, int interpolationMode = 2) global native

; Add a translation point at the current camera position
; time: time in seconds when this point occurs
; easeIn: ease in at the start of interpolation
; easeOut: ease out at the end of interpolation
; interpolationMode: 0=None, 1=Linear, 2=CubicHermite (default)
; Returns: index of the added point
int Function FCSE_AddTranslationPointAtCamera(float time, bool easeIn = false, bool easeOut = false, int interpolationMode = 2) global native

; Add a rotation point with specified pitch and yaw
; time: time in seconds when this point occurs
; pitch: pitch rotation
; yaw: yaw rotation
; easeIn: ease in at the start of interpolation
; easeOut: ease out at the end of interpolation
; interpolationMode: 0=None, 1=Linear, 2=CubicHermite (default)
; Returns: index of the added point
int Function FCSE_AddRotationPoint(float time, float pitch, float yaw, bool easeIn = false, bool easeOut = false, int interpolationMode = 2) global native

; Add a rotation point using camera-to-reference direction plus offset
; time: time in seconds when this point occurs
; reference: the object reference to track
; offsetPitch: pitch offset from camera-to-reference direction
; offsetYaw: yaw offset from camera-to-reference direction. A value of 0 means looking directly at the reference.
; easeIn: ease in at the start of interpolation
; easeOut: ease out at the end of interpolation
; interpolationMode: 0=None, 1=Linear, 2=CubicHermite (default)
; Returns: index of the added point
int Function FCSE_AddRotationPointAtRef(float time, ObjectReference reference, float offsetPitch, float offsetYaw, bool easeIn = false, bool easeOut = false, int interpolationMode = 2) global native

; Add a rotation point at the current camera rotation
; time: time in seconds when this point occurs
; easeIn: ease in at the start of interpolation
; easeOut: ease out at the end of interpolation
; interpolationMode: 0=None, 1=Linear, 2=CubicHermite (default)
; Returns: index of the added point
int Function FCSE_AddRotationPointAtCamera(float time, bool easeIn = false, bool easeOut = false, int interpolationMode = 2) global native

; Start recording camera movements to the timeline
Function FCSE_StartRecording() global native

; Stop recording camera movements
Function FCSE_StopRecording() global native

; Edit an existing translation point
; index: index of the point to edit
; time: new time in seconds
; posX, posY, posZ: new position coordinates
; easeIn: ease in at the start of interpolation
; easeOut: ease out at the end of interpolation
; interpolationMode: 0=None, 1=Linear, 2=CubicHermite (default)
; Returns: new index of the point after potential re-sorting
int Function FCSE_EditTranslationPoint(int index, float time, float posX, float posY, float posZ, bool easeIn, bool easeOut, int interpolationMode = 2) global native

; Edit an existing rotation point
; index: index of the point to edit
; time: new time in seconds
; pitch: new pitch rotation (in radians or degrees depending on configuration)
; yaw: new yaw rotation (in radians or degrees depending on configuration)
; easeIn: ease in at the start of interpolation
; easeOut: ease out at the end of interpolation
; interpolationMode: 0=None, 1=Linear, 2=CubicHermite (default)
; Returns: new index of the point after potential re-sorting
int Function FCSE_EditRotationPoint(int index, float time, float pitch, float yaw, bool easeIn, bool easeOut, int interpolationMode = 2) global native

; Remove a translation point from the timeline
; index: index of the point to remove
Function FCSE_RemoveTranslationPoint(int index) global native

; Remove a rotation point from the timeline
; index: index of the point to remove
Function FCSE_RemoveRotationPoint(int index) global native

; Clear the entire camera timeline
; notifyUser: whether to show a notification to the user
Function FCSE_ClearTimeline(bool notifyUser = true) global native

; Get the number of translation points in the timeline
; Returns: number of translation points
int Function FCSE_GetTranslationPointCount() global native

; Get the number of rotation points in the timeline
; Returns: number of rotation points
int Function FCSE_GetRotationPointCount() global native

; Start playback with advanced options
; speed: playback speed multiplier (only used if useDuration=false)
; useDuration: if true, plays timeline over duration seconds
;              if false, plays timeline with speed as speed multiplier
; duration: total duration in seconds for entire timeline (only used if useDuration=true)
; globalEaseIn: apply ease-in at the start of entire playback
; globalEaseOut: apply ease-out at the end of entire playback
Function FCSE_StartPlayback(float speed = 1.0, bool globalEaseIn = false, bool globalEaseOut = false, bool useDuration = false, float duration = 0.0) global native

; Stop playback of the camera timeline
Function FCSE_StopPlayback() global native

; Pause the camera timeline playback
Function FCSE_PausePlayback() global native

; Resume the camera timeline playback
Function FCSE_ResumePlayback() global native

; Check if timeline playback is currently running
; Returns: true if playing, false otherwise
bool Function FCSE_IsPlaybackRunning() global native

; Check if timeline playback is currently paused
; Returns: true if paused, false otherwise
bool Function FCSE_IsPlaybackPaused() global native

; Enable or disable user rotation control during playback
; allow: true to allow user rotation, false to disable
Function FCSE_AllowUserRotation(bool allow) global native

; Check if user rotation is currently allowed during playback
; Returns: true if user can control rotation, false otherwise
bool Function FCSE_IsUserRotationAllowed() global native

; Adds camera timeline imported from filePath at timeOffset to the current timeline.
; filePath: Relative path from Data folder (e.g., "SKSE/Plugins/MyTimeline.dat")
; timeOffset: Time offset to add to all imported point times (default 0.0)
; Returns: true if successful, false otherwise
bool Function FCSE_AddTimelineFromFile(string filePath, float timeOffset = 0.0) global native

; Export the camera timeline to a file
; filePath: Relative path from Data folder (e.g., "SKSE/Plugins/MyTimeline.dat")
; Returns: true if successful, false otherwise
bool Function FCSE_ExportTimeline(string filePath) global native
